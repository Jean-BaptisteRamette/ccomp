#ifndef CHASM_CONTROL_FLOW_CONTEXT_HPP
#define CHASM_CONTROL_FLOW_CONTEXT_HPP


#include <unordered_set>
#include <stack>

#include <chasm/ds/paths.hpp>


namespace chasm::ds
{
	class analysis_path : public path
	{
	public:
		explicit analysis_path(arch::addr);

		//
		// set/get analysis of path ended
		//
		void mark_end();
		[[nodiscard]] bool ended() const;

	private:
		bool ended_ {};
	};

	class analysis_procedure : public procedure
	{
	public:
		explicit analysis_procedure(arch::addr);

		[[nodiscard]] procedure to_procedure();
		[[nodiscard]] analysis_path& current_path();
		void add_path(arch::addr);

	private:
		// TODO: this
		std::vector<analysis_path> analysis_paths;
	};

	class control_flow_context
	{
	public:
		control_flow_context() = default;
		~control_flow_context() = default;

		[[nodiscard]] analysis_path& current_path();
		[[nodiscard]] analysis_procedure& analyzed_procedure();

		void path_push(arch::addr path_entrypoint);
		void path_pop();

		void callstack_push(arch::addr proc_entrypoint);
		void callstack_pop();

		[[nodiscard]] bool was_visited(arch::addr path_entrypoint) const;
		[[nodiscard]] bool inside_procedure() const;

	private:
		std::stack<analysis_procedure> callstack;
		std::stack<analysis_path> paths;

		std::unordered_set<arch::addr> visited_entrypoints;
	};
}


#endif //CHASM_CONTROL_FLOW_CONTEXT_HPP
