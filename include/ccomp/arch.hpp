#ifndef CCOMP_ARCH_HPP
#define CCOMP_ARCH_HPP


#include <unordered_map>
#include <string_view>


namespace ccomp::arch
{
	using size_type = uint16_t;
	using opcode = size_type;
	using imm    = size_type;
	using addr   = size_type;
	using reg    = uint8_t;

	enum imm_format
	{
		imm4  =  4,
		imm8  =  8,
		imm12 = 12
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
		// general 8-bit registers
		//
		reg_rx,

		//
		// immediate 8-bit value
		//
		imm8,

		//
		// indirect value (has to be immediate)
		//
		imm8_indirect
	};

	[[nodiscard]] constexpr bool imm_matches_format(imm v, imm_format width)
	{
		return v < (1 << width);
	}

	[[nodiscard]] constexpr uint16_t make_operands_mask(std::initializer_list<operand_type> operand_types)
	{
		uint16_t mask = 0;
		uint16_t shift = 0;

		for (operand_type op : operand_types)
		{
			mask |= (static_cast<uint8_t>(op) << shift);
			shift += 3;
		}

		return mask;
	}

	constexpr auto MAX_OPERANDS = 3;

	constexpr auto MASK_ADD_R8_R8 = make_operands_mask({ operand_type::reg_rx, operand_type::reg_rx });
	constexpr auto MASK_ADD_R8_I8 = make_operands_mask({ operand_type::reg_rx, operand_type::imm8 });
	constexpr auto MASK_SUB_R8_R8 = make_operands_mask({ operand_type::reg_rx, operand_type::reg_rx });
	constexpr auto MASK_SUBA_R8_R8 = make_operands_mask({ operand_type::reg_rx, operand_type::reg_rx });
	constexpr auto MASK_OR_R8_R8 = make_operands_mask({ operand_type::reg_rx, operand_type::reg_rx });
	constexpr auto MASK_AND_R8_R8 = make_operands_mask({ operand_type::reg_rx, operand_type::reg_rx });
	constexpr auto MASK_XOR_R8_R8 = make_operands_mask({ operand_type::reg_rx, operand_type::reg_rx });
	constexpr auto MASK_SHR_R8 = make_operands_mask({ operand_type::reg_rx });
	constexpr auto MASK_SHL_R8 = make_operands_mask({ operand_type::reg_rx });
	constexpr auto MASK_SHR_R8_R8 = make_operands_mask({ operand_type::reg_rx, operand_type::reg_rx });
	constexpr auto MASK_SHL_R8_R8 = make_operands_mask({ operand_type::reg_rx, operand_type::reg_rx });
	constexpr auto MASK_RDUMP_R8 = make_operands_mask({ operand_type::reg_rx });
	constexpr auto MASK_RLOAD_R8 = make_operands_mask({ operand_type::reg_rx });
	constexpr auto MASK_MOV_R8_I8 = make_operands_mask({ operand_type::reg_rx, operand_type::imm8 });
	constexpr auto MASK_MOV_R8_R8 = make_operands_mask({ operand_type::reg_rx, operand_type::reg_rx });
	constexpr auto MASK_MOV_AR_R8 = make_operands_mask({ operand_type::reg_ar, operand_type::reg_rx });
	constexpr auto MASK_ADD_AR_R8 = make_operands_mask({ operand_type::reg_ar, operand_type::reg_rx });
	constexpr auto MASK_MOV_DT_R8 = make_operands_mask({ operand_type::reg_dt, operand_type::reg_rx });
	constexpr auto MASK_MOV_ST_R8 = make_operands_mask({ operand_type::reg_st, operand_type::reg_rx });
	constexpr auto MASK_MOV_R8_DT = make_operands_mask({ operand_type::reg_rx, operand_type::reg_dt });
	constexpr auto MASK_DRAW_R8_R8_I8 = make_operands_mask({ operand_type::reg_rx, operand_type::reg_rx, operand_type::imm8 });
	constexpr auto MASK_RAND_R8_I8 = make_operands_mask({ operand_type::reg_rx, operand_type::imm8 });
	constexpr auto MASK_BCD_R8 = make_operands_mask({ operand_type::reg_rx });
	constexpr auto MASK_WKEY_R8 = make_operands_mask({ operand_type::reg_rx });
	constexpr auto MASK_SKE_R8 = make_operands_mask({ operand_type::reg_rx });
	constexpr auto MASK_SKNE_R8 = make_operands_mask({ operand_type::reg_rx });
	constexpr auto MASK_SE_R8_I8 = make_operands_mask({ operand_type::reg_rx, operand_type::imm8 });
	constexpr auto MASK_SNE_R8_I8 = make_operands_mask({ operand_type::reg_rx, operand_type::imm8 });
	constexpr auto MASK_SE_R8_R8 = make_operands_mask({ operand_type::reg_rx, operand_type::reg_rx });
	constexpr auto MASK_SNE_R8_R8 = make_operands_mask({ operand_type::reg_rx, operand_type::reg_rx });
	constexpr auto MASK_INC_R8 = make_operands_mask({ operand_type::reg_rx });


	opcode _00E0();
	opcode _00EE();

	opcode _5XY0(reg rx, reg ry);
	opcode _8XY0(reg rx, reg ry);
	opcode _8XY1(reg rx, reg ry);
	opcode _8XY2(reg rx, reg ry);
	opcode _8XY3(reg rx, reg ry);
	opcode _8XY4(reg rx, reg ry);
	opcode _8XY5(reg rx, reg ry);
	opcode _8XY7(reg rx, reg ry);
	opcode _9XY0(reg rx, reg ry);

	opcode _8XY6(reg rx, reg ry);
	opcode _8XYE(reg rx, reg ry);

	opcode _8X06(reg rx);
	opcode _8X0E(reg rx);

	opcode _3XNN(reg reg_index, imm imm8);
	opcode _4XNN(reg reg_index, imm imm8);
	opcode _6XNN(reg reg_index, imm imm8);
	opcode _7XNN(reg reg_index, imm imm8);
	opcode _CXNN(reg reg_index, imm imm8);

	opcode _EX9E(reg rx);
	opcode _EXA1(reg rx);
	opcode _FX07(reg rx);
	opcode _FX0A(reg rx);
	opcode _FX29(reg rx);
	opcode _FX15(reg rx);
	opcode _FX18(reg rx);
	opcode _FX1E(reg rx);
	opcode _FX33(reg rx);
	opcode _FX55(reg rx);
	opcode _FX65(reg rx);

	opcode _1NNN(/* TODO */);
	opcode _2NNN(/* TODO */);
	opcode _BNNN(/* TODO */);
	opcode _ANNN(/* TODO */);

	opcode _DXYN(reg rx, reg ry, imm imm4);

}

#endif //CCOMP_ARCH_HPP
