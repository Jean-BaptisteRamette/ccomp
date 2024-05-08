#include <chasm/ds/paths.hpp>


namespace chasm::ds
{
	path::path(arch::addr start)
		: start_addr(start)
	{}

	arch::addr path::addr_start() const
	{
		return start_addr;
	}

	arch::addr path::addr_end() const
	{
		return start_addr + instructions_count() * sizeof(arch::opcode);
	}

	size_t path::instructions_count() const
	{
		return disassembly.size();
	}

	std::string path::symbolic(size_t instruction_index) const
	{
		return disassembly[instruction_index];
	}

	bool path::operator<(const path &other) const
	{
		return addr_start() < other.addr_start();
	}

	std::set<path> procedure::get_paths() const
	{
		return ordered_paths;
	}

	void procedure::insert_path(path p)
	{
		ordered_paths.insert(std::move(p));
	}

	procedure::procedure(arch::addr ep)
		: ep(ep)
	{}

	arch::addr procedure::entrypoint() const
	{
		return ep;
	}

	bool procedure::operator<(const procedure &other) const
	{
		return entrypoint() < other.entrypoint();
	}
}
