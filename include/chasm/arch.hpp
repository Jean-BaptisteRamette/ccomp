#ifndef CHASM_ARCH_HPP
#define CHASM_ARCH_HPP


#include <initializer_list>
#include <unordered_map>
#include <string_view>
#include <algorithm>
#include <array>


namespace chasm::arch
{
	using size_type = uint16_t;
	using opcode = size_type;
	using imm    = size_type;
	using addr   = size_type;
	using reg    = uint8_t;
	using operands_mask = size_type;


	enum instruction_id
	{
		ADD,
		AND,
		BCD,
		CALL,
		CLS,
		DRAW,
		EXIT,
		HIGH,
		INC,
		JMP,
		LDF,
		LDFS,
		LOADRPL,
		LOW,
		MOV,
		OR,
		RAND,
		RDUMP,
		RET,
		RLOAD,
		SAVERPL,
		SCRD,
		SCRL,
		SCRR,
		SE,
		SHL,
		SHR,
		SKE,
		SKNE,
		SNE,
		SUB,
		SUBA,
		SWP,
		WKEY,
		XOR,

		count
	};

	constexpr std::array<std::string_view, instruction_id::count> mnemonics = {
			"add",
			"and",
			"bcd",
			"call",
			"cls",
			"draw",
			"exit",
			"high",
			"inc",
			"jmp",
			"ldf",
			"ldfs",
			"loadrpl",
			"low",
			"mov",
			"or",
			"rand",
			"rdump",
			"ret",
			"rload",
			"saverpl",
			"scrd",
			"scrl",
			"scrr",
			"se",
			"shl",
			"shr",
			"ske",
			"skne",
			"sne",
			"sub",
			"suba",
			"swp",
			"wkey",
			"xor"
	};

	static_assert(std::ranges::is_sorted(mnemonics));

	constexpr instruction_id to_instruction_id(const std::string_view& mnemonic)
	{
		auto it = std::ranges::lower_bound(mnemonics, mnemonic);

		if (it == std::end(mnemonics) || *it != mnemonic)
			return instruction_id::count;

		return static_cast<instruction_id>(std::distance(mnemonics.begin(), it));
	}

	constexpr bool has_mnemonic(const std::string_view& instruction)
	{
		return std::ranges::binary_search(mnemonics, instruction);
	}


	enum imm_format
	{
		fmt_imm4  =  4,
		fmt_imm8  =  8,
		fmt_imm12 = 12,
		fmt_imm16 = 16
	};

	enum class operand_type
	{
		//
		// special registers
		//
		reg_ar = 1,
		reg_st,
		reg_dt,

		//
		// general 8-bit register
		//
		reg_rx,

		//
		// immediate value, can be up to 12 bits for immediate addresses
		//
		immediate,

		//
		// operands of type: [addr] -> (r0 + addr)
		//
		address_indirect,

		//
		// address type for jmp labels, procedures and sprites
		//
		address
	};

	constexpr auto BITSHIFT_OP_MASK = std::bit_width(static_cast<uint8_t>(operand_type::address_indirect));

	constexpr size_type MAX_PROGRAM_SIZE = 0x1000 - 0x200;
	constexpr auto MAX_SPRITE_ROWS = 15;

	struct sprite
	{
		std::array<uint8_t, MAX_SPRITE_ROWS> data;
		uint8_t row_count;
	};

	[[nodiscard]] constexpr bool imm_matches_format(imm v, imm_format width)
	{
		return v < (1 << width);
	}

	[[nodiscard]] constexpr operands_mask make_operands_mask(std::initializer_list<operand_type> operand_types)
	{
		operands_mask mask = 0;
		uint16_t shift = 0;

		for (operand_type op : operand_types)
		{
			mask |= (static_cast<uint8_t>(op) << shift);
			shift += BITSHIFT_OP_MASK;
		}

		return mask;
	}

	constexpr auto MAX_OPERANDS = 3;

	constexpr auto MASK_NONE      = make_operands_mask({});
	constexpr auto MASK_R8_R8     = make_operands_mask({ operand_type::reg_rx, operand_type::reg_rx });
	constexpr auto MASK_R8_IMM    = make_operands_mask({ operand_type::reg_rx, operand_type::immediate });
	constexpr auto MASK_R8        = make_operands_mask({ operand_type::reg_rx });
	constexpr auto MASK_AR_R8     = make_operands_mask({ operand_type::reg_ar, operand_type::reg_rx });
	constexpr auto MASK_AR_IMM    = make_operands_mask({ operand_type::reg_ar, operand_type::immediate });
	constexpr auto MASK_AR_ADDR   = make_operands_mask({ operand_type::reg_ar, operand_type::address });
	constexpr auto MASK_DT_R8     = make_operands_mask({ operand_type::reg_dt, operand_type::reg_rx });
	constexpr auto MASK_ST_R8     = make_operands_mask({ operand_type::reg_st, operand_type::reg_rx });
	constexpr auto MASK_R8_DT     = make_operands_mask({ operand_type::reg_rx, operand_type::reg_dt });
	constexpr auto MASK_IMM       = make_operands_mask({ operand_type::immediate });
	constexpr auto MASK_ADDR      = make_operands_mask({ operand_type::address });
	constexpr auto MASK_ADDR_REL  = make_operands_mask({ operand_type::address_indirect });
	constexpr auto MASK_R8_R8_IMM = make_operands_mask({ operand_type::reg_rx, operand_type::reg_rx, operand_type::immediate });

	//
	// There isn't any instruction of type (r8, r8, addr), but we use this so the draw
	// instruction can accept a sprite as an operand. In the end, the sprite will just be converted to its size.
	//
	constexpr auto MASK_R8_R8_ADDR = make_operands_mask({ operand_type::reg_rx, operand_type::reg_rx, operand_type::address });


	namespace enc
	{

#define ENCODE_dXYN(id, rx, ry, N) ((id << 12u) | (rx << 8u) | (ry << 4u) | N)
#define ENCODE_dXNN(id, rx, NN)    ((id << 12u) | (rx << 8u) | NN)
#define ENCODE_dNNN(id, NNN)       ((id << 12u) | NNN)

#define EXPAND(x) x
#define GET_OVERLOAD(_1, _2, _3, _4, OVERLOAD, ...) OVERLOAD
#define ENCODE(...) EXPAND(GET_OVERLOAD(__VA_ARGS__, ENCODE_dXYN, ENCODE_dXNN, ENCODE_dNNN)(__VA_ARGS__))

		constexpr opcode _00CN(imm imm4) { return ENCODE(0, 0xC, imm4); }
		constexpr opcode _5XY0(reg rx, reg ry) { return ENCODE(0x5, rx, ry, 0x0); }
		constexpr opcode _8XY0(reg rx, reg ry) { return ENCODE(0x8, rx, ry, 0x0); }
		constexpr opcode _8XY1(reg rx, reg ry) { return ENCODE(0x8, rx, ry, 0x1); }
		constexpr opcode _8XY2(reg rx, reg ry) { return ENCODE(0x8, rx, ry, 0x2); }
		constexpr opcode _8XY3(reg rx, reg ry) { return ENCODE(0x8, rx, ry, 0x3); }
		constexpr opcode _8XY4(reg rx, reg ry) { return ENCODE(0x8, rx, ry, 0x4); }
		constexpr opcode _8XY5(reg rx, reg ry) { return ENCODE(0x8, rx, ry, 0x5); }
		constexpr opcode _8XY7(reg rx, reg ry) { return ENCODE(0x8, rx, ry, 0x7); }
		constexpr opcode _9XY0(reg rx, reg ry) { return ENCODE(0x9, rx, ry, 0x0); }
		constexpr opcode _8XY6(reg rx, reg ry) { return ENCODE(0x8, rx, ry, 0x06); }
		constexpr opcode _8XYE(reg rx, reg ry) { return ENCODE(0x8, rx, ry, 0x0E); }

		constexpr opcode _8X06(reg rx) { return ENCODE(0x8, rx, 0x06); }
		constexpr opcode _8X0E(reg rx) { return ENCODE(0x8, rx, 0x0E); }

		constexpr opcode _3XNN(reg rx, imm imm8) { return ENCODE(0x3, rx, imm8); }
		constexpr opcode _4XNN(reg rx, imm imm8) { return ENCODE(0x4, rx, imm8); }
		constexpr opcode _6XNN(reg rx, imm imm8) { return ENCODE(0x6, rx, imm8); }
		constexpr opcode _7XNN(reg rx, imm imm8) { return ENCODE(0x7, rx, imm8); }
		constexpr opcode _CXNN(reg rx, imm imm8) { return ENCODE(0xC, rx, imm8); }


		constexpr opcode _EX9E(reg rx) { return ENCODE(0xE, rx, 0x9E); }
		constexpr opcode _EXA1(reg rx) { return ENCODE(0xE, rx, 0xA1); }
		constexpr opcode _FX07(reg rx) { return ENCODE(0xF, rx, 0x07); }
		constexpr opcode _FX0A(reg rx) { return ENCODE(0xF, rx, 0x0A); }
		constexpr opcode _FX15(reg rx) { return ENCODE(0xF, rx, 0x15); }
		constexpr opcode _FX18(reg rx) { return ENCODE(0xF, rx, 0x18); }
		constexpr opcode _FX1E(reg rx) { return ENCODE(0xF, rx, 0x1E); }
		constexpr opcode _FX29(reg rx) { return ENCODE(0xF, rx, 0x29); }
		constexpr opcode _FX30(reg rx) { return ENCODE(0xF, rx, 0x30); }
		constexpr opcode _FX33(reg rx) { return ENCODE(0xF, rx, 0x33); }
		constexpr opcode _FX55(reg rx) { return ENCODE(0xF, rx, 0x55); }
		constexpr opcode _FX65(reg rx) { return ENCODE(0xF, rx, 0x65); }
		constexpr opcode _FX75(reg rx) { return ENCODE(0xF, rx, 0x75); }
		constexpr opcode _FX85(reg rx) { return ENCODE(0xF, rx, 0x85); }

		constexpr opcode _1NNN(arch::imm imm12) { return ENCODE(0x1, imm12); }
		constexpr opcode _2NNN(arch::imm imm12) { return ENCODE(0x2, imm12); }
		constexpr opcode _BNNN(arch::imm imm12) { return ENCODE(0xB, imm12); }
		constexpr opcode _ANNN(arch::imm imm12) { return ENCODE(0xA, imm12); }
		constexpr opcode _DXYN(reg rx, reg ry, imm imm4) { return ENCODE(0xD, rx, ry, imm4); }
	}
}

#endif //CHASM_ARCH_HPP
