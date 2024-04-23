#include <iostream>
#include <chasm/ds/disassembly_view.hpp>


namespace chasm::ds
{
	disassembly_view::disassembly_view(const chasm::ds::disassembler &ds_instance)
		: paths(ds_instance.code_paths())
	{}

	void disassembly_view::print() const
	{
		for (const auto& p : paths)
		{
			std::cout << std::format(".loc_{:X}:   # Code path: 0x{:04X} - 0x{:04X}", p.addr_start(), p.addr_start(), p.addr_end()) << std::endl;

			for (size_t i = 0; i < p.instructions_count(); ++i)
				std::cout << std::format("\t{}", p.symbolic(i))  << std::endl;
		}
	}
}