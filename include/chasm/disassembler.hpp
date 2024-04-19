#ifndef CHASM_DISASSEMBLER_HPP
#define CHASM_DISASSEMBLER_HPP


#include <unordered_map>
#include <vector>
#include <string>

#include <chasm/arch.hpp>


namespace chasm::ds
{
	/*!
	 * Example output:
	 *
	 * proc sub_ADDRESS
	 *     mov r1, 2
	 *     jmp @loc_ADDRESS
	 *
	 * .loc_ADDRESS:
	 *     ret
	 * endp sub_ADDRESS
	 *
	 * .main:
	 *     mov r0, 0
	 *     mov r1, 1
	 *     add r0, r1
	 *     call $sub_ADDRESS
	 *
	 *     mov r2, 6
	 *     mov ar, 0xABCD
	 *     draw r2, r2, 7
	 */

	/*!
	 *
	 *  1NNN
	 *  2NNN
	 *  ANNN
	 *  BNNN
	 *
	 *  3XNN
	 *  4XNN
	 *  6XNN
	 *  7XNN
	 *  CXNN
	 *
	 *  5XY0
	 *  8XY0
	 *  8XY1
	 *  8XY2
	 *  8XY3
	 *  8XY4
	 *  8XY5
	 *  8XY6
	 *  8XY7
	 *  8XYE
	 *  9XY0
	 *
	 *  EX9E
	 *  EXA1
	 *  FX07
	 *  FX0A
	 *  FX15
	 *  FX18
	 *  FX1E
	 *  FX29
	 *  FX33
	 *  FX55
	 *  FX65
	 *
	 *  DXYN
	 *
	 *  00EE
	 *  00E0
	 *
	 *  - First nibble can't be an operand
	 *  - get the first nibble
	 *
	 */



	const std::unordered_map<uint8_t, arch::operands_mask> nibble2operands = {
			{ 0x0, arch::MASK_NONE },

			{ 0x1, arch::MASK_ADDR },
			{ 0x2, arch::MASK_ADDR },
			{ 0xA, arch::MASK_ADDR },
			{ 0xB, arch::MASK_ADDR },

			{ 0x3, arch::MASK_R8_IMM },
			{ 0x4, arch::MASK_R8_IMM },
			{ 0x6, arch::MASK_R8_IMM },
			{ 0x7, arch::MASK_R8_IMM },
			{ 0xC, arch::MASK_R8_IMM },

			{ 0x5, arch::MASK_R8_R8 },
			{ 0x8, arch::MASK_R8_R8 },
			{ 0x9, arch::MASK_R8_R8 },

			{ 0xE, arch::MASK_R8 },
			{ 0xF, arch::MASK_R8 },

			{ 0xD, arch::MASK_R8_R8_IMM },
	};

	struct decoded_instruction
	{

	};

	class disassembler
	{
	public:
		explicit disassembler(std::vector<uint8_t> from_bytes);
		~disassembler() = default;

		disassembler(disassembler&) = delete;
		disassembler(disassembler&&) = delete;
		disassembler& operator=(disassembler&) = delete;
		disassembler& operator=(disassembler&&) = delete;

		std::vector<decoded_instruction> instructions();

	private:
		std::vector<uint8_t> binary;
	};
}


#endif //CHASM_DISASSEMBLER_HPP
