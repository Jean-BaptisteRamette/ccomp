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

	void path::mark_end()
	{
		ended_ = true;
	}

	bool path::ended() const
	{
		return ended_;
	}

	size_t path::instructions_count() const
	{
		return disassembly.size();
	}

	std::string path::symbolic(size_t instruction_index) const
	{
		return disassembly[instruction_index];
	}
}
