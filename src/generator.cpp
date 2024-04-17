#include <span>
#include <fstream>
#include <algorithm>

#include <chasm/generator.hpp>
#include <chasm/options.hpp>
#include <chasm/arch.hpp>
#include <chasm/log.hpp>


namespace chasm
{
	void generate_symbols_file(const std::filesystem::path& path,
							   const std::unordered_map<std::string, arch::addr>& mapping)
	{
		std::ofstream os(path);

		if (!os)
		{
			log::error("Could not open file \"{}\" to write symbols mapping.", path.string());
			return;
		}

		auto sort_pred = [](const std::pair<std::string, arch::addr>& a,
				            const std::pair<std::string, arch::addr>& b)
		{
			return a.second < b.second;
		};

		//
		// Sort from lowest to highest address
		//
		std::vector<std::pair<std::string, arch::addr>> elems(mapping.begin(), mapping.end());
		std::sort(elems.begin(), elems.end(), sort_pred);

		for (const auto& [symbol, addr] : elems)
			os << std::format("{:#06x} --> {}", addr, symbol) << std::endl;

		log::info("{} symbols mapping written to \"{}\".", mapping.size(), path.string());
	}

	void warn_super_instruction(const ast::instruction_statement& instruction)
	{
		log::warn("Instruction {} at {} is a SuperCHIP-8 instruction but flag \"super\" was not provided.",
				  instruction.mnemonic.to_string(),
				  to_string(instruction.mnemonic.source_location));
	}

	[[nodiscard]]
	arch::reg operand2reg(const ast::instruction_operand& operand)
	{
		const char id = operand.reg_name()[1];

		if (std::isdigit(id))
			return id - '0';

		return 0xA + (id - 'a');
	}

	[[nodiscard]]
	arch::operands_mask make_operands_mask(const ast::instruction_statement& instruction)
	{
		if (instruction.operands.size() > arch::MAX_OPERANDS)
			throw assembler_error("Instruction \"{}\" at {} has {} operands "
								  "but CHIP-8 instructions can have up to {} operands.",
								  instruction.operands.size(),
								  arch::MAX_OPERANDS);

		arch::operands_mask mask = 0;
		uint16_t shift = 0;

		for (const auto& operand : instruction.operands)
		{
			mask |= (static_cast<uint8_t>(operand.arch_type()) << shift);
			shift += arch::BITSHIFT_OP_MASK;
		}

		return mask;
	}

	void ensure_operands_count(const ast::instruction_statement& inst, std::initializer_list<int> expected_counts)
	{
		if (!std::ranges::contains(expected_counts, inst.operands.size()))
			throw generator_exception::invalid_operands_count(inst, expected_counts);
	}

	void ensure_operands_count(const ast::instruction_statement& inst, int expected_count)
	{
		if (inst.operands.size() != expected_count)
			throw generator_exception::invalid_operands_count(inst, { expected_count });
	}

	std::vector<uint8_t> generator::generate(const ast::abstract_tree& ast)
	{
		for (const auto& branch : ast.branches())
			branch->accept(*this);

		post_visit();

		return binary;
	}

	void generator::post_visit()
	{
		//
		// Add sprites to the end of the code
		//
		for (const auto& [name, sprite] : sprites)
		{
			register_symbol_addr(name);

			binary.append_range(std::span(sprite.data.begin(), sprite.row_count));

			const bool misaligned = binary.size() % sizeof(arch::opcode) != 0;

			if (misaligned && options::has_flag("pad-sprites"))
				binary.push_back(0x00);
		}

		const auto base_addr = options::arg<arch::addr>("relocate");

		//
		// Apply jmp/call patches that could not be encoded directly
		//
		for (const auto& [location, sym] : patches)
		{
			const uintptr_t relocated = base_addr + sym_addresses[sym];

			if (relocated > std::numeric_limits<arch::addr>::max())
				throw assembler_error("Symbol \"{}\" relocated to address {:x} which is out of the chip8's memory range.\n"
									  "Assembler cannot generate address patch at {:x}",
									  sym,
									  relocated,
									  location * sizeof(arch::opcode));

			binary[location + 0] |= ((static_cast<arch::addr>(relocated) & 0x0F00) >> 8);
			binary[location + 1] |= ((static_cast<arch::addr>(relocated) & 0x00FF));
		}

		if (options::has_flag("symbols"))
			generate_symbols_file(options::arg<std::string>("symbols"), sym_addresses);
	}

	void generator::emit_opcode(arch::opcode opcode)
	{
		binary.push_back((opcode & 0xFF00) >> 8);
		binary.push_back((opcode & 0x00FF));
	}

	void generator::emit_opcodes(const std::vector<arch::opcode>& opcodes)
	{
		for (auto opcode : opcodes)
			emit_opcode(opcode);
	}

	void generator::visit(const ast::procedure_statement& procedure)
	{
		register_symbol_addr(procedure.name_beg.to_string());

		current_proc_name = procedure.name_beg.to_string();

		for (const auto& inner : procedure.inner_statements)
			inner->accept(*this);

		current_proc_name = "";
	}

	void generator::visit(const ast::instruction_statement& instruction)
	{
		const auto mnemonic = instruction.mnemonic.to_string();

		if (mnemonic_encoders.contains(mnemonic))
		{
			auto encoder = mnemonic_encoders.at(mnemonic);
			emit_opcode((this->*encoder)(instruction));
		}
		else if (pseudo_mnemonic_encoders.contains(mnemonic))
		{
			auto encoder = pseudo_mnemonic_encoders.at(mnemonic);
			emit_opcodes((this->*encoder)(instruction));
		}
		else if (super_mnemonic_encoders.contains(mnemonic))
		{
			if (!options::has_flag("super"))
				warn_super_instruction(instruction);

			auto encoder = super_mnemonic_encoders.at(mnemonic);
			emit_opcode((this->*encoder)(instruction));
		}
	}

	void generator::visit(const ast::define_statement& define)
	{
		register_constant(define.identifier.to_string(), define.value.to_integer());
	}

	void generator::visit(const ast::sprite_statement& sprite_statement)
	{
		register_sprite(sprite_statement.identifier.to_string(), sprite_statement.sprite);
	}

	void generator::visit(const ast::label_statement& label)
	{
		register_symbol_addr(current_proc_name + "." + label.identifier.to_string());

		for (const auto& inner : label.inner_statements)
			inner->accept(*this);
	}

	void generator::visit(const ast::raw_statement& statement)
	{
		emit_opcode(operand2imm(statement.opcode, arch::fmt_imm16));
	}

	void generator::register_constant(std::string &&symbol, arch::imm value)
	{
		//
		// The sanitizer has already made sure it is not the same scope,
		// so we can safely overwrite the previous definition
		//
		constants[std::move(symbol)] = value;
	}

	void generator::register_sprite(std::string&& symbol, const arch::sprite& sprite)
	{
		if (sprites.contains(symbol))
			throw assembler_error("Generator found an already defined sprite \"{}\", this should have been caught by the sanitizer.", symbol);

		sprites[std::move(symbol)] = sprite;
	}

	void generator::register_symbol_addr(std::string symbol)
	{
		if (sym_addresses.contains(symbol))
			throw assembler_error("Generator found an already existing symbol \"{}\", this should have been caught by the sanitizer.", symbol);

		sym_addresses[std::move(symbol)] = binary.size();
	}

	void generator::register_patch_location(std::string&& symbol)
	{
		patches.push_back({
			.location = static_cast<arch::addr>(binary.size()),
			.sym = std::move(symbol)
		});
	}

	arch::imm generator::operand2imm(const token& token, arch::imm_format imm_width) const
	{
		arch::imm imm = 0;

		if (token.type == token_type::numerical)
			imm = token.to_integer();
		else
		{
			const auto identifier = token.to_string();

			if (constants.contains(identifier))
				imm = constants.at(identifier);
			else if (sprites.contains(identifier))
				imm = sprites.at(identifier).row_count;
		}

		if (!arch::imm_matches_format(imm, imm_width))
			throw generator_exception::invalid_immediate_format(imm, imm_width);

		return imm;
	}

	arch::imm generator::operand2imm(const ast::instruction_operand& operand, arch::imm_format imm) const
	{
		return operand2imm(operand.operand, imm);
	}

	arch::opcode generator::encode_add(const ast::instruction_statement& add)
	{
		ensure_operands_count(add, 2);

		switch (make_operands_mask(add))
		{
			case arch::MASK_R8_R8:
				return arch::_8XY4(
						operand2reg(add.operands[0]),
						operand2reg(add.operands[1]));

			case arch::MASK_R8_IMM:
				return arch::_7XNN(
						operand2reg(add.operands[0]),
						operand2imm(add.operands[1]));

			case arch::MASK_AR_R8:
				return arch::_FX1E(operand2reg(add.operands[1]));

			default:
				throw generator_exception::invalid_operand_type(add);
		}
	}

	arch::opcode generator::encode_sub(const ast::instruction_statement& sub)
	{
		ensure_operands_count(sub, 2);

		if (make_operands_mask(sub) == arch::MASK_R8_R8)
			return arch::_8XY5(
					operand2reg(sub.operands[0]),
					operand2reg(sub.operands[1]));

		throw generator_exception::invalid_operand_type(sub);
	}

	arch::opcode generator::encode_suba(const ast::instruction_statement& suba)
	{
		ensure_operands_count(suba, 2);

		if (make_operands_mask(suba) == arch::MASK_R8_R8)
			return arch::_8XY7(
					operand2reg(suba.operands[0]),
					operand2reg(suba.operands[1]));

		throw generator_exception::invalid_operand_type(suba);
	}

	arch::opcode generator::encode_or(const ast::instruction_statement& or_)
	{
		ensure_operands_count(or_, 2);

		if (make_operands_mask(or_) == arch::MASK_R8_R8)
			return arch::_8XY1(
					operand2reg(or_.operands[0]),
					operand2reg(or_.operands[1]));

		throw generator_exception::invalid_operand_type(or_);
	}

	arch::opcode generator::encode_and(const ast::instruction_statement& and_)
	{
		ensure_operands_count(and_, 2);

		if (make_operands_mask(and_) == arch::MASK_R8_R8)
			return arch::_8XY2(
					operand2reg(and_.operands[0]),
					operand2reg(and_.operands[1]));

		throw generator_exception::invalid_operand_type(and_);
	}

	arch::opcode generator::encode_xor(const ast::instruction_statement& xor_)
	{
		ensure_operands_count(xor_, 2);

		if (make_operands_mask(xor_) == arch::MASK_R8_R8)
			return arch::_8XY3(
					operand2reg(xor_.operands[0]),
					operand2reg(xor_.operands[1]));

		throw generator_exception::invalid_operand_type(xor_);
	}

	arch::opcode generator::encode_shr(const ast::instruction_statement& shr)
	{
		ensure_operands_count(shr, { 1, 2 });

		switch (make_operands_mask(shr))
		{
			case arch::MASK_R8:
				return arch::_8X06(operand2reg(shr.operands[0]));

			case arch::MASK_R8_R8:
				return arch::_8XY6(
						operand2reg(shr.operands[0]),
						operand2reg(shr.operands[1]));

			default:
				throw generator_exception::invalid_operand_type(shr);
		}
	}

	arch::opcode generator::encode_shl(const ast::instruction_statement& shl)
	{
		ensure_operands_count(shl, { 1, 2 });

		switch (make_operands_mask(shl))
		{
			case arch::MASK_R8:
				return arch::_8X0E(operand2reg(shl.operands[0]));

			case arch::MASK_R8_R8:
				return arch::_8XYE(
						operand2reg(shl.operands[0]),
						operand2reg(shl.operands[1]));

			default:
				throw generator_exception::invalid_operand_type(shl);
		}
	}

	arch::opcode generator::encode_rdump(const ast::instruction_statement& rdump)
	{
		ensure_operands_count(rdump, 1);

		if (make_operands_mask(rdump) == arch::MASK_R8)
			return arch::_FX55(operand2reg(rdump.operands[0]));

		throw generator_exception::invalid_operand_type(rdump);
	}

	arch::opcode generator::encode_rload(const ast::instruction_statement& rload)
	{
		ensure_operands_count(rload, 1);

		if (make_operands_mask(rload) == arch::MASK_R8)
			return arch::_FX65(operand2reg(rload.operands[0]));

		throw generator_exception::invalid_operand_type(rload);
	}

	arch::opcode generator::encode_mov(const ast::instruction_statement& mov)
	{
		ensure_operands_count(mov, 2);

		switch (make_operands_mask(mov))
		{
			case arch::MASK_R8_R8: return arch::_8XY0(operand2reg(mov.operands[0]), operand2reg(mov.operands[1]));
			case arch::MASK_R8_IMM: return arch::_6XNN(operand2reg(mov.operands[0]), operand2imm(mov.operands[1]));
			case arch::MASK_R8_DT: return arch::_FX07(operand2reg(mov.operands[0]));
			case arch::MASK_DT_R8: return arch::_FX15(operand2reg(mov.operands[1]));
			case arch::MASK_ST_R8: return arch::_FX18(operand2reg(mov.operands[1]));
			case arch::MASK_AR_R8: return arch::_FX29(operand2reg(mov.operands[1]));
			case arch::MASK_AR_IMM: return arch::_ANNN(operand2imm(mov.operands[1], arch::fmt_imm12));

			case arch::MASK_AR_ADDR:
			{
				register_patch_location(mov.operands[1].operand.to_string());
				return arch::_ANNN(0);
			}

			default:
				throw generator_exception::invalid_operand_type(mov);
		}
	}

	arch::opcode generator::encode_draw(const ast::instruction_statement& draw)
	{
		ensure_operands_count(draw, 3);

		switch (make_operands_mask(draw))
		{
			case arch::MASK_R8_R8_IMM:
			{
				const auto regX = operand2reg(draw.operands[0]);
				const auto regY = operand2reg(draw.operands[1]);
				const auto imm4 = operand2imm(draw.operands[2], arch::fmt_imm4);

				if (imm4 == 0 && !options::has_flag("super"))
					warn_super_instruction(draw);

				return arch::_DXYN(regX, regY, imm4);
			}
			case arch::MASK_R8_R8_ADDR:
			{
				//
				// draw r0, r1, #s  == draw r0, r1, sizeof(s)
				//
				const auto regX   = operand2reg(draw.operands[0]);
				const auto regY   = operand2reg(draw.operands[1]);
				const auto& third = draw.operands[2];

				if (third.is_sprite())
					return arch::_DXYN(regX, regY, operand2imm(third, arch::fmt_imm4));
			}
		}

		throw generator_exception::invalid_operand_type(draw);
	}

	arch::opcode generator::encode_cls(const ast::instruction_statement& cls)
	{
		ensure_operands_count(cls, 0);
		return 0x00E0;
	}

	arch::opcode generator::encode_rand(const ast::instruction_statement& rand)
	{
		ensure_operands_count(rand, 2);

		if (make_operands_mask(rand) == arch::MASK_R8_IMM)
			return arch::_CXNN(
					operand2reg(rand.operands[0]),
					operand2imm(rand.operands[1]));

		throw generator_exception::invalid_operand_type(rand);
	}

	arch::opcode generator::encode_bcd(const ast::instruction_statement& bcd)
	{
		ensure_operands_count(bcd, 1);

		if (make_operands_mask(bcd) == arch::MASK_R8)
			return arch::_FX33(operand2reg(bcd.operands[0]));

		throw generator_exception::invalid_operand_type(bcd);
	}

	arch::opcode generator::encode_wkey(const ast::instruction_statement& wkey)
	{
		ensure_operands_count(wkey, 1);

		if (make_operands_mask(wkey) == arch::MASK_R8)
			return arch::_FX0A(operand2reg(wkey.operands[0]));

		throw generator_exception::invalid_operand_type(wkey);
	}

	arch::opcode generator::encode_ske(const ast::instruction_statement& ske)
	{
		ensure_operands_count(ske, 1);

		if (make_operands_mask(ske) == arch::MASK_R8)
			return arch::_EX9E(operand2reg(ske.operands[0]));

		throw generator_exception::invalid_operand_type(ske);
	}

	arch::opcode generator::encode_skne(const ast::instruction_statement& skne)
	{
		ensure_operands_count(skne, 1);

		if (make_operands_mask(skne) == arch::MASK_R8)
			return arch::_EXA1(operand2reg(skne.operands[0]));

		throw generator_exception::invalid_operand_type(skne);
	}

	arch::opcode generator::encode_ret(const ast::instruction_statement& ret)
	{
		ensure_operands_count(ret, 0);
		return 0x00EE;
	}

	arch::opcode generator::encode_jmp(const ast::instruction_statement& jmp)
	{
		ensure_operands_count(jmp, 1);

		switch (make_operands_mask(jmp))
		{
			case arch::MASK_ADDR:
				if (jmp.operands[0].is_label())
				{
					// jmp @label
					register_patch_location(current_proc_name + "." + jmp.operands[0].operand.to_string());

					return arch::_1NNN(0);
				}

			// jmp [offset]
			case arch::MASK_ADDR_REL:
				return arch::_BNNN(operand2imm(jmp.operands[0], arch::fmt_imm12));

			default:
				throw generator_exception::invalid_operand_type(jmp);
		}
	}

	arch::opcode generator::encode_call(const ast::instruction_statement& call)
	{
		ensure_operands_count(call, 1);

		switch (make_operands_mask(call))
		{
			case arch::MASK_ADDR:
				if (call.operands[0].is_procedure())
				{
					// call $function
					register_patch_location(call.operands[0].operand.to_string());

					return arch::_2NNN(0);
				}

			default:
				break;
		}

		throw generator_exception::invalid_operand_type(call);
	}

	arch::opcode generator::encode_se(const ast::instruction_statement& se)
	{
		ensure_operands_count(se, 2);

		switch (make_operands_mask(se))
		{
			case arch::MASK_R8_R8:
				return arch::_5XY0(
						operand2reg(se.operands[0]),
						operand2reg(se.operands[1]));

			case arch::MASK_R8_IMM:
				return arch::_3XNN(
						operand2reg(se.operands[0]),
						operand2imm(se.operands[1]));

			default:
				throw generator_exception::invalid_operand_type(se);
		}
	}

	arch::opcode generator::encode_sne(const ast::instruction_statement& sne)
	{
		ensure_operands_count(sne, 2);

		switch (make_operands_mask(sne))
		{
			case arch::MASK_R8_R8:
				return arch::_9XY0(
						operand2reg(sne.operands[0]),
						operand2reg(sne.operands[1]));

			case arch::MASK_R8_IMM:
				return arch::_4XNN(
						operand2reg(sne.operands[0]),
						operand2imm(sne.operands[1]));

			default:
				throw generator_exception::invalid_operand_type(sne);
		}
	}

	arch::opcode generator::encode_inc(const ast::instruction_statement& inc)
	{
		ensure_operands_count(inc, 1);

		if (make_operands_mask(inc) == arch::MASK_R8)
			return arch::_7XNN(operand2reg(inc.operands[0]), 1);

		throw generator_exception::invalid_operand_type(inc);
	}

	arch::opcode generator::encode_exit(const ast::instruction_statement& exit)
	{
		ensure_operands_count(exit, 0);
		return 0x00FD;
	}

	arch::opcode generator::encode_scrd(const ast::instruction_statement& scrd)
	{
		ensure_operands_count(scrd, 1);

		if (make_operands_mask(scrd) == arch::MASK_IMM)
			return arch::_00CN(operand2imm(scrd.operands[0], arch::fmt_imm4));

		throw generator_exception::invalid_operand_type(scrd);
	}

	arch::opcode generator::encode_scrl(const ast::instruction_statement& scrl)
	{
		ensure_operands_count(scrl, 0);
		return 0x00FC;
	}

	arch::opcode generator::encode_scrr(const ast::instruction_statement& scrr)
	{
		ensure_operands_count(scrr, 0);
		return 0x00FB;
	}

	arch::opcode generator::encode_high(const ast::instruction_statement& high)
	{
		ensure_operands_count(high, 0);
		return 0x00FF;
	}

	arch::opcode generator::encode_low(const ast::instruction_statement& low)
	{
		ensure_operands_count(low, 0);
		return 0x00FE;
	}

	std::vector<arch::opcode> generator::encode_swp(const ast::instruction_statement& swp)
	{
		ensure_operands_count(swp, 2);

		if (make_operands_mask(swp) == arch::MASK_R8_R8)
		{
			std::vector<arch::opcode> opcodes;

			// We can use the xor swapping trick to avoid clobbering a temporary register
			//
			// xor rX, rY
			// xor rY, rX
			// xor rX, rY
			const auto rX = operand2reg(swp.operands[0]);
			const auto rY = operand2reg(swp.operands[1]);

			opcodes.push_back(arch::_8XY3(rX, rY));
			opcodes.push_back(arch::_8XY3(rY, rX));
			opcodes.push_back(arch::_8XY3(rX, rY));

			return opcodes;
		}

		throw generator_exception::invalid_operand_type(swp);
	}
}