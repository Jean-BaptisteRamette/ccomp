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

	analysis_procedure::analysis_procedure(arch::addr ep)
		: procedure(ep)
	{}

	analysis_path& control_flow_context::current_path()
	{
		if (inside_procedure())
			return analyzed_procedure().current_path();

		return paths.top();
	}

	analysis_procedure& control_flow_context::analyzed_procedure()
	{
		return callstack.top();
	}

	void control_flow_context::path_push(arch::addr path_entrypoint)
	{
		if (inside_procedure())
			analyzed_procedure().add_path(path_entrypoint);

		paths.emplace(path_entrypoint);

		visited_entrypoints.insert(path_entrypoint);
	}

	void control_flow_context::path_pop()
	{
		paths.pop();
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

	bool control_flow_context::was_visited(arch::addr path_entrypoint) const
	{
		return std::ranges::contains(visited_entrypoints, path_entrypoint);
	}

	bool control_flow_context::inside_procedure() const
	{
		return !callstack.empty();
	}
}