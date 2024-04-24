#include <chasm/ds/disassembler.hpp>
#include <chasm/ds/paths.hpp>
#include <chasm/options.hpp>
#include <chasm/arch.hpp>


namespace chasm::ds
{
	disassembler::disassembler(std::vector<uint8_t> from_bytes)
		: binary(std::move(from_bytes)),
		  pm(),
		  current_path(nullptr),
		  code_base(options::arg<arch::addr>("relocate"))
	{
		//
		// Add initial code path at entry point
		//
		pm.try_add_path(code_base);

		while (pm.has_pending())
		{
			auto p = path(pm.next_unprocessed());

			ds_path(p);

			pm.add_processed(std::move(p));
		}
	}

	std::vector<path> disassembler::code_paths() const
	{
		return pm.paths();
	}

	void disassembler::ds_path(path& p)
	{
		current_path = &p;

		while (!current_path->ended())
			ds_next_instruction();
	}

	void disassembler::ds_next_instruction()
	{
		///
		/// The paths manager only manipulates "in-memory" addresses, most of the time offset=0x200
		/// whereas the disassembler works with "disk" addresses, so offset 0x200 maps to address (file offset) 0 on disk
		///
		const arch::addr at = current_path->addr_end() - code_base;

		if (at + 1 >= binary.size() || at>= binary.size())
			throw chasm_exception("Unexpected end of bytes while decoding instruction during disassembly at address 0x{:04X}", at);

		const auto opcode = static_cast<arch::opcode>(binary[at] << 8 | binary[at + 1]);

		const auto n1 = static_cast<uint8_t>((opcode & 0xF000) >> 12);
		const auto n2 = static_cast<uint8_t>((opcode & 0x0F00) >> 8);
		const auto n3 = static_cast<uint8_t>((opcode & 0x00F0) >> 4);
		const auto n4 = static_cast<uint8_t>(opcode & 0x000F);
		const auto imm8 = static_cast<arch::imm>(opcode & 0x00FF);
		const auto addr = static_cast<arch::addr>(opcode & 0x0FFF);

		switch (n1)
		{
			case 0x0:
			{
				switch (opcode)
				{
					case 0x00E0: ds_cls(); return;
					case 0x00EE: ds_ret(); return;
					case 0x00FB: ds_scrr(); return;
					case 0x00FC: ds_scrl(); return;
					case 0x00FD: ds_exit(); return;
					case 0x00FE: ds_low(); return;
					case 0x00FF: ds_high(); return;

					default:
					{
						if (n3 == 0xC)
						{
							ds_scrd();
							return;
						}
					}
				}

				break;
			}

			case 0x1: ds_jmp(addr); return;
			case 0x2: ds_call(addr); return;
			case 0xA: ds_mov_ar_addr(addr); return;
			case 0xB: ds_jmp_indirect(addr); return;

			case 0x3: ds_se_r8_imm(n2, imm8); return;
			case 0x4: ds_sne_r8_imm(n2, imm8); return;
			case 0x6: ds_mov_r8_imm(n2, imm8); return;
			case 0x7: ds_add_r8_imm(n2, imm8); return;
			case 0xC: ds_rand_r8_imm(n2, imm8); return;

			case 0x5:
				if (n4 == 0)
				{
					ds_se_r8_r8(n2, n3);
					return;
				}

				break;

			case 0x8:
				switch (n4)
				{
					case 0x0: ds_mov_r8_r8(n2, n3); return;
					case 0x1: ds_or_r8_r8(n2, n3); return;
					case 0x2: ds_and_r8_r8(n2, n3); return;
					case 0x3: ds_xor_r8_r8(n2, n3); return;
					case 0x4: ds_add_r8_r8(n2, n3); return;
					case 0x5: ds_sub_r8_r8(n2, n3); return;
					case 0x6: ds_shl_r8_r8(n2, n3); return;
					case 0x7: ds_suba_r8_r8(n2, n3); return;
					case 0xE: ds_shr_r8_r8(n2, n3); return;

					default: break;
				}

				break;

			case 0x9:
				if (n4 == 0)
				{
					ds_sne_r8_r8(n2, n3);
					return;
				}

			case 0xE:
				if (imm8 == 0x9E)
				{
					ds_ske_r8(n2);
					return;
				}

			case 0xF:
				switch (imm8)
				{
					case 0x07: ds_mov_r8_dt(n2); return;
					case 0x15: ds_mov_dt_r8(n2); return;
					case 0x18: ds_mov_st_r8(n2); return;
					case 0x1E: ds_add_ar_r8(n2); return;
					case 0x29: ds_ldf_r8(n2); return;
					case 0x30: ds_ldfs_r8(n2); return;
					case 0x55: ds_rdump_r8(n2); return;
					case 0x65: ds_rload_r8(n2); return;
					case 0x75: ds_saverpl_r8(n2); return;
					case 0x85: ds_loadrpl_r8(n2); return;

					default: break;
				}

				break;

			case 0xD: ds_draw_r8_r8_imm(n2, n3, n4); return;

			default:
				break;
		}

		throw disassembly_exception::decoding_error(opcode, at);
	}
	
	void disassembler::emit(arch::instruction_id id)
	{
		current_path->add_instruction(id);
	}

	void disassembler::ds_cls()
	{
		emit(arch::instruction_id::CLS);
	}

	void disassembler::ds_ret()
	{
		emit(arch::instruction_id::RET);

		current_path->mark_end();
	}

	void disassembler::ds_scrr()
	{
		emit(arch::instruction_id::SCRR);
	}

	void disassembler::ds_scrl()
	{
		emit(arch::instruction_id::SCRL);
	}

	void disassembler::ds_exit()
	{
		emit(arch::instruction_id::EXIT);

		current_path->mark_end();
	}

	void disassembler::ds_low()
	{
		emit(arch::instruction_id::LOW);
	}

	void disassembler::ds_high()
	{
		emit(arch::instruction_id::HIGH);
	}

	void disassembler::ds_scrd()
	{
		emit(arch::instruction_id::SCRD);
	}

	void disassembler::ds_call(arch::addr subroutine_addr)
	{
		emit(arch::instruction_id::CALL);

		pm.try_add_path(subroutine_addr);
	}

	void disassembler::ds_jmp(arch::addr location)
	{
		emit(arch::instruction_id::JMP);

		current_path->mark_end();

		if (location != current_path->addr_start())
			pm.try_add_path(location);
	}

	void disassembler::ds_mov_ar_addr(arch::addr addr)
	{
		emit(arch::instruction_id::MOV);
	}

	void disassembler::ds_jmp_indirect(arch::addr offset)
	{
		emit(arch::instruction_id::JMP);

		current_path->mark_end();
	}

	void disassembler::ds_se_r8_imm(arch::reg reg, arch::imm imm)
	{
		emit(arch::instruction_id::SE);
	}

	void disassembler::ds_sne_r8_imm(arch::reg reg, arch::imm imm)
	{
		emit(arch::instruction_id::SNE);
	}

	void disassembler::ds_mov_r8_imm(arch::reg reg, arch::imm imm)
	{
		emit(arch::instruction_id::MOV);
	}

	void disassembler::ds_add_r8_imm(arch::reg reg, arch::imm imm)
	{
		if (imm == 1)
			emit(arch::instruction_id::INC);
		else
			emit(arch::instruction_id::ADD);
	}

	void disassembler::ds_rand_r8_imm(arch::reg reg, arch::imm imm)
	{
		emit(arch::instruction_id::RAND);
	}

	void disassembler::ds_se_r8_r8(arch::reg reg1, arch::reg reg2)
	{
		emit(arch::instruction_id::SE);
	}

	void disassembler::ds_mov_r8_r8(arch::reg reg1, arch::reg reg2)
	{
		emit(arch::instruction_id::MOV);
	}

	void disassembler::ds_or_r8_r8(arch::reg reg1, arch::reg reg2)
	{
		emit(arch::instruction_id::OR);
	}

	void disassembler::ds_and_r8_r8(arch::reg reg1, arch::reg reg2)
	{
		emit(arch::instruction_id::AND);
	}

	void disassembler::ds_xor_r8_r8(arch::reg reg1, arch::reg reg2)
	{
		emit(arch::instruction_id::XOR);
	}

	void disassembler::ds_add_r8_r8(arch::reg reg1, arch::reg reg2)
	{
		emit(arch::instruction_id::ADD);
	}

	void disassembler::ds_sub_r8_r8(arch::reg reg1, arch::reg reg2)
	{
		emit(arch::instruction_id::SUB);
	}

	void disassembler::ds_shl_r8_r8(arch::reg reg1, arch::reg reg2)
	{
		emit(arch::instruction_id::SHL);
	}

	void disassembler::ds_suba_r8_r8(arch::reg reg1, arch::reg reg2)
	{
		emit(arch::instruction_id::SUBA);
	}

	void disassembler::ds_shr_r8_r8(arch::reg reg1, arch::reg reg2)
	{
		emit(arch::instruction_id::SHR);
	}

	void disassembler::ds_sne_r8_r8(arch::reg reg1, arch::reg reg2)
	{
		emit(arch::instruction_id::SNE);
	}

	void disassembler::ds_ske_r8(arch::reg reg)
	{
		emit(arch::instruction_id::SKE);
	}

	void disassembler::ds_mov_r8_dt(arch::reg)
	{
		emit(arch::instruction_id::MOV);
	}

	void disassembler::ds_mov_dt_r8(arch::reg)
	{
		emit(arch::instruction_id::MOV);
	}

	void disassembler::ds_mov_st_r8(arch::reg)
	{
		emit(arch::instruction_id::MOV);
	}

	void disassembler::ds_add_ar_r8(arch::reg)
	{
		emit(arch::instruction_id::ADD);
	}

	void disassembler::ds_ldf_r8(arch::reg)
	{
		emit(arch::instruction_id::LDF);
	}

	void disassembler::ds_ldfs_r8(arch::reg)
	{
		emit(arch::instruction_id::LDFS);
	}

	void disassembler::ds_rdump_r8(arch::reg)
	{
		emit(arch::instruction_id::RDUMP);
	}

	void disassembler::ds_rload_r8(arch::reg)
	{
		emit(arch::instruction_id::RLOAD);
	}

	void disassembler::ds_saverpl_r8(arch::reg)
	{
		emit(arch::instruction_id::SAVERPL);
	}

	void disassembler::ds_loadrpl_r8(arch::reg)
	{
		emit(arch::instruction_id::LOADRPL);
	}

	void disassembler::ds_draw_r8_r8_imm(arch::reg reg1, arch::reg reg2, arch::imm imm)
	{
		emit(arch::instruction_id::DRAW);
	}
}
