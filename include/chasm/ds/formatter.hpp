#ifndef CHASM_FORMATTER_HPP
#define CHASM_FORMATTER_HPP

#include <format>
#include <string>

#include <chasm/arch.hpp>


namespace chasm::ds::formatter
{
#define F_ADDR_REL "[0x{:4X}]"
#define F_ADDR "0x{:3X}"
#define F_IMM "0x{:X}"
#define F_R8 "r{:x}"
#define F_AR "ar"
#define F_DT "dt"
#define F_ST "st"

	template<std::integral ...Args>
	std::string format(arch::instruction_id id, arch::operands_mask mask, Args...args)
	{
		const auto s = [=]() -> std::string
		{
			switch (mask)
			{
				case arch::MASK_R8_R8: return F_R8 ", " F_R8;
				case arch::MASK_R8_IMM: return F_R8 ", " F_IMM;
				case arch::MASK_R8: return F_R8;
				case arch::MASK_AR_R8: return F_AR ", " F_R8;
				case arch::MASK_AR_IMM: return F_AR ", " F_IMM;
				case arch::MASK_AR_ADDR: return F_AR ", " F_ADDR;
				case arch::MASK_DT_R8: return F_DT ", " F_R8;
				case arch::MASK_ST_R8: return F_ST ", " F_R8;
				case arch::MASK_R8_DT: return F_R8 ", " F_DT;
				case arch::MASK_IMM: return F_IMM;
				case arch::MASK_ADDR: return F_ADDR;
				case arch::MASK_ADDR_REL: return F_ADDR_REL;
				case arch::MASK_R8_R8_IMM: return F_R8 ", " F_R8 ", " F_IMM;

				default:
					return {};
			}
		}();

		return std::vformat("{} " + s, std::make_format_args(arch::mnemonics[id], args...));
	}
}


#endif //CHASM_FORMATTER_HPP
