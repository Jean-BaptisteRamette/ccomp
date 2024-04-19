#include <chasm/disassembler.hpp>


namespace chasm::ds
{
	disassembler::disassembler(std::vector<uint8_t> from_bytes)
		: binary(std::move(from_bytes))
	{}

	std::vector<decoded_instruction> disassembler::instructions()
	{
		return {};
	}
}
