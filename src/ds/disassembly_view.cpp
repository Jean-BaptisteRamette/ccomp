#include <iostream>
#include <chasm/ds/disassembly_view.hpp>


namespace chasm::ds
{
	disassembly_view::disassembly_view(const chasm::ds::disassembler &ds_instance)
		: paths(ds_instance.code_paths())
	{}

	std::string disassembly_view::to_string() const
	{
		std::string o;
		o.reserve(1024);

		for (const auto& p : paths)
		{
			o += std::format(".loc_{:X}:   ;; Code path: 0x{:04X} - 0x{:04X}\n", p.addr_start(), p.addr_start(), p.addr_end());

			for (size_t i = 0; i < p.instructions_count(); ++i)
				o += std::format("    {}\n", p.symbolic(i));
		}

		return o;
	}
}