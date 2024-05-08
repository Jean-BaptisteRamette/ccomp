#include <chasm/ds/disassembly_graph.hpp>



namespace chasm::ds
{
	void disassembly_graph::insert_proc(procedure p)
	{
		procedures.insert(std::move(p));
	}

	void disassembly_graph::insert_path(path p)
	{
		paths.insert(std::move(p));
	}

	std::set<procedure> disassembly_graph::get_procedures() const
	{
		return procedures;
	}

	std::set<path> disassembly_graph::get_paths() const
	{
		return paths;
	}
}


