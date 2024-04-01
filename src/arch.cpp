#include <ccomp/arch.hpp>


namespace ccomp::arch
{
#define ENCODE_dXYN(id, rx, ry, N) ((id << 12u) | (rx << 8u) | (ry << 4u) | N)
#define ENCODE_dXNN(id, rx, NN)    ((id << 12u) | (rx << 8u) | NN)
#define ENCODE_dNNN(id, NNN)       ((id << 12u) | NNN)

#define GET_OVERLOAD(_1, _2, _3, _4, OVERLOAD, ...) OVERLOAD
#define ENCODE(...) GET_OVERLOAD(__VA_ARGS__, ENCODE_dXYN, ENCODE_dXNN, ENCODE_dNNN)(__VA_ARGS__)


	opcode _00E0() { return 0x00E0; }
	opcode _00EE() { return 0x00EE; }

	opcode _5XY0(reg rx, reg ry) { return ENCODE(0x5, rx, ry, 0x0); }
	opcode _8XY0(reg rx, reg ry) { return ENCODE(0x8, rx, ry, 0x0); }
	opcode _8XY1(reg rx, reg ry) { return ENCODE(0x8, rx, ry, 0x1); }
	opcode _8XY2(reg rx, reg ry) { return ENCODE(0x8, rx, ry, 0x2); }
	opcode _8XY3(reg rx, reg ry) { return ENCODE(0x8, rx, ry, 0x3); }
	opcode _8XY4(reg rx, reg ry) { return ENCODE(0x8, rx, ry, 0x4); }
	opcode _8XY5(reg rx, reg ry) { return ENCODE(0x8, rx, ry, 0x5); }
	opcode _8XY7(reg rx, reg ry) { return ENCODE(0x8, rx, ry, 0x7); }
	opcode _9XY0(reg rx, reg ry) { return ENCODE(0x9, rx, ry, 0x0); }

	opcode _8XY6(reg rx, reg ry) { return ENCODE(0x8, rx, ry, 0x06); }
	opcode _8XYE(reg rx, reg ry) { return ENCODE(0x8, rx, ry, 0x0E); }

	opcode _8X06(reg rx) { return ENCODE(0x8, rx, 0x06); }
	opcode _8X0E(reg rx) { return ENCODE(0x8, rx, 0x0E); }

	opcode _3XNN(reg rx, imm imm8) { return ENCODE(0x3, rx, imm8); }
	opcode _4XNN(reg rx, imm imm8) { return ENCODE(0x4, rx, imm8); }
	opcode _6XNN(reg rx, imm imm8) { return ENCODE(0x6, rx, imm8); }
	opcode _7XNN(reg rx, imm imm8) { return ENCODE(0x7, rx, imm8); }
	opcode _CXNN(reg rx, imm imm8) { return ENCODE(0xC, rx, imm8); }


	opcode _EX9E(reg rx) { return ENCODE(0xE, rx, 0x9E); }
	opcode _EXA1(reg rx) { return ENCODE(0xE, rx, 0xA1); }
	opcode _FX07(reg rx) { return ENCODE(0xF, rx, 0x07); }
	opcode _FX0A(reg rx) { return ENCODE(0xF, rx, 0x0A); }
	opcode _FX29(reg rx) { return ENCODE(0xF, rx, 0x29); }
	opcode _FX15(reg rx) { return ENCODE(0xF, rx, 0x15); }
	opcode _FX18(reg rx) { return ENCODE(0xF, rx, 0x18); }
	opcode _FX1E(reg rx) { return ENCODE(0xF, rx, 0x1E); }
	opcode _FX33(reg rx) { return ENCODE(0xF, rx, 0x33); }
	opcode _FX55(reg rx) { return ENCODE(0xF, rx, 0x55); }
	opcode _FX65(reg rx) { return ENCODE(0xF, rx, 0x65); }

	opcode _1NNN(arch::imm imm12) { return 0; }
	opcode _2NNN(arch::imm imm12) { return 0; }
	opcode _BNNN(arch::imm imm12) { return 0; }
	opcode _ANNN(arch::imm imm12) { return 0; }

	opcode _DXYN(reg rx, reg ry, imm imm4) { return ENCODE(0xD, rx, ry, imm4); }
}