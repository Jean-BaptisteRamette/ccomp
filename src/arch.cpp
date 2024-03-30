#include <ccomp/arch.hpp>


namespace ccomp::arch
{
	namespace
	{
		// TODO: drop this map and just use defined masks
		const std::unordered_map<std::string_view, unsigned short> operands_count_ = {
				{ "add",   2 },
				{ "sub",   2 },
				{ "suba",  2 },
				{ "dec",   1 },
				{ "inc",   1 },
				{ "or",    2 },
				{ "and",   2 },
				{ "xor",   2 },
				{ "shr",   1 },
				{ "shl",   1 },
				{ "rdump", 1 },
				{ "rload", 1 },
				{ "mov",   2 },
				{ "swp",   2 },
				{ "draw",  3 },
				{ "cls",   0 },
				{ "rand",  2 },
				{ "bcd",   1 },
				{ "wkey",  1 },
				{ "ske",   1 },
				{ "snke",  1 },
				{ "ret",   0 },
				{ "jmp",   1 },
				{ "call",  1 },
				{ "se",    2 },
				{ "sne",   2 }
		};
	}

	unsigned char operands_count(std::string_view mnemonic)
	{
		return operands_count_.at(mnemonic);
	}

	unsigned char regname2regindex(std::string_view regname)
	{
		// TODO
		return 0;
	}

#define ENCODE_dXYN(id, rx, ry, N) ((id << 12u) | (rx << 8u) | (ry << 4u) | N)
#define ENCODE_dXNN(id, rx, NN)    ((id << 12u) | (rx << 8u) | NN)
#define ENCODE_dNNN(id, NNN)       ((id << 12u) | NNN)

#define GET_OVERLOAD(_1, _2, _3, _4, OVERLOAD, ...) OVERLOAD
#define ENCODE(...) GET_OVERLOAD(__VA_ARGS__, ENCODE_dXYN, ENCODE_dXNN, ENCODE_dNNN)(__VA_ARGS__)


	opcode _8XY4(uint8_t reg_index1, uint8_t reg_index2) { return ENCODE(8, reg_index1, reg_index2, 4); }
	opcode _8XY5(uint8_t reg_index1, uint8_t reg_index2) { return ENCODE(8, reg_index1, reg_index2, 5); }

	opcode _7XNN(uint8_t reg_index, uint8_t imm)
	{
		return ENCODE(7, reg_index, imm);
	}
}