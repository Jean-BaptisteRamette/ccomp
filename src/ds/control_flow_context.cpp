#include <chasm/ds/control_flow_context.hpp>


namespace chasm::ds
{
	analysis_path::analysis_path(arch::addr start)
		: path(start)
	{}

	void analysis_path::mark_end()
	{
		ended_ = true;
	}

	bool analysis_path::ended() const
	{
		return ended_;
	}

	analysis_procedure::analysis_procedure(arch::addr ep)
			: procedure(ep)
	{}

	analysis_path& analysis_procedure::current_path()
	{
		return analysis_paths.back();
	}

	void analysis_procedure::add_path(arch::addr p)
	{
		analysis_paths.emplace_back(p);
	}

	procedure analysis_procedure::to_procedure()
	{
		auto ret = procedure(this->entrypoint());

		for (analysis_path& p : analysis_paths)
			ret.insert_path(std::move(p));

		return ret;
	}

	const std::vector<analysis_path>& analysis_procedure::analyzed_paths() const
	{
		return analysis_paths;
	}

	analysis_path& control_flow_context::analyzed_path()
	{
		if (inside_procedure())
			return analyzed_procedure().current_path();

		return paths.back();
	}

	analysis_procedure& control_flow_context::analyzed_procedure()
	{
		return callstack.top();
	}

	void control_flow_context::path_push(arch::addr path_entrypoint)
	{
		if (inside_procedure())
			analyzed_procedure().add_path(path_entrypoint);

		paths.emplace_back(path_entrypoint);

		visited_entrypoints.insert(path_entrypoint);
	}

	void control_flow_context::path_pop()
	{
		paths.pop_back();
	}

	void control_flow_context::callstack_push(arch::addr proc_entrypoint)
	{
		callstack.emplace(proc_entrypoint);

		path_push(proc_entrypoint);
	}

	void control_flow_context::callstack_pop()
	{
		path_pop();
		callstack.pop();
	}

	bool control_flow_context::was_visited(arch::addr address) const
	{
		//
		// check against known labels and procedures entrypoints
		//
		if (visited_entrypoints.contains(address))
			return true;

		// In the following scenario:
		// 		.main:
		// 		   mov r0, 0
		// 		.label:
		//         mov r0, 0
		// 		   jmp @label
		//
		// we don't want to queue ".label" path for analysis as it was already analyzed by the main path
		// however, if the alignement is not the same, that means it was not analyzed yet, so queue it
		//

		auto path_has_address = [=](const analysis_path& p) -> bool
		{
			const bool in_range = address > p.addr_start() && address < p.addr_end();
			const bool same_alignement = arch::is_aligned(address) == arch::is_aligned(p.addr_start());

			return in_range && same_alignement;
		};

		if (inside_procedure())
		{
			const auto current_proc = callstack.top();
			return std::ranges::any_of(current_proc.analyzed_paths(), path_has_address);
		}

		return std::ranges::any_of(paths, path_has_address);
	}

	bool control_flow_context::inside_procedure() const
	{
		return !callstack.empty();
	}
}