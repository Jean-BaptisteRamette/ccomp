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
		return regname[1] - '0';
	}

#define ENCODE_dXYN(id, rx, ry, N) ((id << 12u) | (rx << 8u) | (ry << 4u) | N)
#define ENCODE_dXNN(id, rx, NN)    ((id << 12u) | (rx << 8u) | NN)
#define ENCODE_dNNN(id, NNN)       ((id << 12u) | NNN)

#define GET_OVERLOAD(_1, _2, _3, _4, OVERLOAD, ...) OVERLOAD
#define ENCODE(...) GET_OVERLOAD(__VA_ARGS__, ENCODE_dXYN, ENCODE_dXNN, ENCODE_dNNN)(__VA_ARGS__)


	opcode _00E0() { return 0x00E0; }
	opcode _00EE() { return 0x00EE; }

	opcode _5XY0(uint8_t reg_index1, uint8_t reg_index2) { return ENCODE(0x5, reg_index1, reg_index2, 0x0); }
	opcode _8XY0(uint8_t reg_index1, uint8_t reg_index2) { return ENCODE(0x8, reg_index1, reg_index2, 0x0); }
	opcode _8XY1(uint8_t reg_index1, uint8_t reg_index2) { return ENCODE(0x8, reg_index1, reg_index2, 0x1); }
	opcode _8XY2(uint8_t reg_index1, uint8_t reg_index2) { return ENCODE(0x8, reg_index1, reg_index2, 0x2); }
	opcode _8XY3(uint8_t reg_index1, uint8_t reg_index2) { return ENCODE(0x8, reg_index1, reg_index2, 0x3); }
	opcode _8XY4(uint8_t reg_index1, uint8_t reg_index2) { return ENCODE(0x8, reg_index1, reg_index2, 0x4); }
	opcode _8XY5(uint8_t reg_index1, uint8_t reg_index2) { return ENCODE(0x8, reg_index1, reg_index2, 0x5); }
	opcode _8XY7(uint8_t reg_index1, uint8_t reg_index2) { return ENCODE(0x8, reg_index1, reg_index2, 0x7); }
	opcode _9XY0(uint8_t reg_index1, uint8_t reg_index2) { return ENCODE(0x9, reg_index1, reg_index2, 0x0); }

	opcode _3XNN(uint8_t reg_index, uint8_t imm) { return ENCODE(0x3, reg_index, imm); }
	opcode _4XNN(uint8_t reg_index, uint8_t imm) { return ENCODE(0x4, reg_index, imm); }
	opcode _6XNN(uint8_t reg_index, uint8_t imm) { return ENCODE(0x6, reg_index, imm); }
	opcode _7XNN(uint8_t reg_index, uint8_t imm) { return ENCODE(0x7, reg_index, imm); }
	opcode _CXNN(uint8_t reg_index, uint8_t imm) { return ENCODE(0xC, reg_index, imm); }

	opcode _EX9E(uint8_t reg_index) { return ENCODE(0xE, reg_index, 0x9E); }
	opcode _EXA1(uint8_t reg_index) { return ENCODE(0xE, reg_index, 0xA1); }
	opcode _FX07(uint8_t reg_index) { return ENCODE(0xF, reg_index, 0x07); }
	opcode _FX0A(uint8_t reg_index) { return ENCODE(0xF, reg_index, 0x0A); }
	opcode _FX29(uint8_t reg_index) { return ENCODE(0xF, reg_index, 0x29); }
	opcode _FX15(uint8_t reg_index) { return ENCODE(0xF, reg_index, 0x15); }
	opcode _FX18(uint8_t reg_index) { return ENCODE(0xF, reg_index, 0x18); }
	opcode _FX1E(uint8_t reg_index) { return ENCODE(0xF, reg_index, 0x1E); }
	opcode _FX33(uint8_t reg_index) { return ENCODE(0xF, reg_index, 0x33); }
	opcode _FX55(uint8_t reg_index) { return ENCODE(0xF, reg_index, 0x55); }
	opcode _FX65(uint8_t reg_index) { return ENCODE(0xF, reg_index, 0x65); }

	opcode _1NNN(/* TODO */) { return 0; }
	opcode _2NNN(/* TODO */) { return 0; }
	opcode _BNNN(/* TODO */) { return 0; }
	opcode _ANNN(/* TODO */) { return 0; }

	opcode _DXYN(uint8_t reg_index1, uint8_t reg_index2, uint8_t imm)
	{
		return ENCODE(0xD, reg_index1, reg_index2, imm);
	}
}