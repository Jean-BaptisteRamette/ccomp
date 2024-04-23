#ifndef CHASM_DISASSEMBLER_HPP
#define CHASM_DISASSEMBLER_HPP


#include <map>
#include <set>
#include <stack>
#include <vector>
#include <string>

#include <chasm/ds/paths_manager.hpp>
#include <chasm/chasm_exception.hpp>
#include <chasm/arch.hpp>


namespace chasm::ds
{
	class disassembler
	{
	public:
		explicit disassembler(std::vector<uint8_t> from_bytes);
		~disassembler() = default;

		disassembler(disassembler&) = delete;
		disassembler(disassembler&&) = delete;
		disassembler& operator=(disassembler&) = delete;
		disassembler& operator=(disassembler&&) = delete;

		/// Get the disassembled code paths
		///
		/// \return
		[[nodiscard]] std::vector<path> code_paths() const;


	private:
		void ds_path(path&);
		void ds_next_instruction();
		void emit(arch::instruction_id);

		void ds_cls();
		void ds_ret();
		void ds_scrr();
		void ds_scrl();
		void ds_exit();
		void ds_low();
		void ds_high();
		void ds_scrd();
		void ds_call(arch::addr subroutine_addr);
		void ds_jmp(arch::addr location);
		void ds_mov_ar_addr(arch::addr addr);
		void ds_jmp_indirect(arch::addr offset);
		void ds_se_r8_imm(arch::reg reg, arch::imm imm);
		void ds_sne_r8_imm(arch::reg reg, arch::imm imm);
		void ds_mov_r8_imm(arch::reg reg, arch::imm imm);
		void ds_add_r8_imm(arch::reg reg, arch::imm imm);
		void ds_rand_r8_imm(arch::reg reg, arch::imm imm);
		void ds_se_r8_r8(arch::reg reg1, arch::reg reg2);
		void ds_mov_r8_r8(arch::reg reg1, arch::reg reg2);
		void ds_or_r8_r8(arch::reg reg1, arch::reg reg2);
		void ds_and_r8_r8(arch::reg reg1, arch::reg reg2);
		void ds_xor_r8_r8(arch::reg reg1, arch::reg reg2);
		void ds_add_r8_r8(arch::reg reg1, arch::reg reg2);
		void ds_sub_r8_r8(arch::reg reg1, arch::reg reg2);
		void ds_shl_r8_r8(arch::reg reg1, arch::reg reg2);
		void ds_suba_r8_r8(arch::reg reg1, arch::reg reg2);
		void ds_shr_r8_r8(arch::reg reg1, arch::reg reg2);
		void ds_sne_r8_r8(arch::reg reg1, arch::reg reg2);
		void ds_ske_r8(arch::reg reg);
		void ds_mov_r8_dt(arch::reg);
		void ds_mov_dt_r8(arch::reg);
		void ds_mov_st_r8(arch::reg);
		void ds_add_ar_r8(arch::reg);
		void ds_ldf_r8(arch::reg);
		void ds_ldfs_r8(arch::reg);
		void ds_rdump_r8(arch::reg);
		void ds_rload_r8(arch::reg);
		void ds_saverpl_r8(arch::reg);
		void ds_loadrpl_r8(arch::reg);
		void ds_draw_r8_r8_imm(arch::reg reg1, arch::reg reg2, arch::imm imm);

	private:
		std::vector<uint8_t> binary;
		paths_manager pm;
		path* current_path;
		arch::addr code_base;
	};

	namespace disassembly_exception
	{
		struct decoding_error : chasm_exception
		{
			explicit decoding_error(arch::opcode invalid, arch::addr where)
				: chasm_exception(
					"Invalid opcode 0x{:04X} at address 0x{:04X} was found while decoding code path",
					invalid,
					where)
			{}
		};
	}
}


#endif //CHASM_DISASSEMBLER_HPP
