#ifndef CHASM_DISASSEMBLY_GRAPH_HPP
#define CHASM_DISASSEMBLY_GRAPH_HPP


#include <set>

#include <chasm/arch.hpp>
#include <chasm/ds/paths.hpp>


namespace chasm::ds
{
	class disassembly_graph
	{
	public:
		disassembly_graph() = default;
		~disassembly_graph() = default;

		disassembly_graph(const disassembly_graph&) = default;
		disassembly_graph(disassembly_graph&&) = default;
		disassembly_graph& operator=(const disassembly_graph&) = default;
		disassembly_graph& operator=(disassembly_graph&&) = default;

		void insert_proc(procedure);
		void insert_path(path);

		[[nodiscard]] std::set<procedure> get_procedures() const;
		[[nodiscard]] std::set<path> get_paths() const;

	private:
		//
		// sorted by entrypoint address
		//
		std::set<procedure> procedures;
		std::set<path> paths;
	};
}


#endif //CHASM_DISASSEMBLY_GRAPH_HPP
