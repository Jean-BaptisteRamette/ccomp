#include <span>
#include <set>
#include <ccomp/generator.hpp>
#include <ccomp/arch.hpp>


namespace ccomp
{
	[[nodiscard]]
	arch::reg operand2reg(const ast::instruction_operand& operand)
	{
		const char id = operand.reg_name()[1];

		if (std::isdigit(id))
			return id - '0';

		return 0xA + (id - 'a');
	}

	[[nodiscard]]
	uint16_t make_operands_mask(const ast::instruction_statement& instruction)
	{
		if (instruction.operands.size() > arch::MAX_OPERANDS)
			throw assembler_error("Instruction \"{}\" at {} has {} operands "
								  "but CHIP-8 instructions can have up to {} operands.",
								  instruction.operands.size(),
								  arch::MAX_OPERANDS);

		uint16_t mask = 0;
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

	std::vector<arch::opcode> generator::generate(const ast::abstract_tree& ast)
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
		for (auto& [name, sprite] : sprites)
		{
			register_symbol_addr(name);
			binary.append_range(std::span(sprite.data.begin(), sprite.row_count));
		}

		//
		// Apply jmp/call patches that could not be encoded directly
		//
		for (const auto& [location, sym] : patches)
			binary[location] |= sym_addresses[sym];
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
			binary.push_back((this->*encoder)(instruction));
		}
		else if (pseudo_mnemonic_encoders.contains(mnemonic))
		{
			auto encoder = pseudo_mnemonic_encoders.at(mnemonic);
			binary.append_range((this->*encoder)(instruction));
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
		binary.push_back(operand2imm(statement.opcode, arch::fmt_imm16));
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

		sym_addresses[std::move(symbol)] = binary.size() * sizeof(arch::opcode);
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
		ensure_operands_count(add, { 2 });

		switch (make_operands_mask(add))
		{
			case arch::MASK_ADD_R8_R8:
				return arch::_8XY4(
						operand2reg(add.operands[0]),
						operand2reg(add.operands[1]));

			case arch::MASK_ADD_R8_I8:
				return arch::_7XNN(
						operand2reg(add.operands[0]),
						operand2imm(add.operands[1]));

			case arch::MASK_ADD_AR_R8:
				return arch::_FX1E(operand2reg(add.operands[1]));

			default:
				throw generator_exception::invalid_operand_type(add);
		}
	}

	arch::opcode generator::encode_sub(const ast::instruction_statement& sub)
	{
		ensure_operands_count(sub, { 2 });

		if (make_operands_mask(sub) == arch::MASK_SUB_R8_R8)
			return arch::_8XY5(
					operand2reg(sub.operands[0]),
					operand2reg(sub.operands[1]));

		throw generator_exception::invalid_operand_type(sub);
	}

	arch::opcode generator::encode_suba(const ast::instruction_statement& suba)
	{
		ensure_operands_count(suba, { 2 });

		if (make_operands_mask(suba) == arch::MASK_SUBA_R8_R8)
			return arch::_8XY7(
					operand2reg(suba.operands[0]),
					operand2reg(suba.operands[1]));

		throw generator_exception::invalid_operand_type(suba);
	}

	arch::opcode generator::encode_or(const ast::instruction_statement& or_)
	{
		ensure_operands_count(or_, { 2 });

		if (make_operands_mask(or_) == arch::MASK_OR_R8_R8)
			return arch::_8XY1(
					operand2reg(or_.operands[0]),
					operand2reg(or_.operands[1]));

		throw generator_exception::invalid_operand_type(or_);
	}

	arch::opcode generator::encode_and(const ast::instruction_statement& and_)
	{
		ensure_operands_count(and_, { 2 });

		if (make_operands_mask(and_) == arch::MASK_AND_R8_R8)
			return arch::_8XY2(
					operand2reg(and_.operands[0]),
					operand2reg(and_.operands[1]));

		throw generator_exception::invalid_operand_type(and_);
	}

	arch::opcode generator::encode_xor(const ast::instruction_statement& xor_)
	{
		ensure_operands_count(xor_, { 2 });

		if (make_operands_mask(xor_) == arch::MASK_XOR_R8_R8)
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
			case arch::MASK_SHR_R8:
				return arch::_8X06(operand2reg(shr.operands[0]));

			case arch::MASK_SHR_R8_R8:
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
			case arch::MASK_SHL_R8:
				return arch::_8X0E(operand2reg(shl.operands[0]));

			case arch::MASK_SHL_R8_R8:
				return arch::_8XYE(
						operand2reg(shl.operands[0]),
						operand2reg(shl.operands[1]));

			default:
				throw generator_exception::invalid_operand_type(shl);
		}
	}

	arch::opcode generator::encode_rdump(const ast::instruction_statement& rdump)
	{
		ensure_operands_count(rdump, { 1 });

		if (make_operands_mask(rdump) == arch::MASK_RDUMP_R8)
			return arch::_FX55(operand2reg(rdump.operands[0]));

		throw generator_exception::invalid_operand_type(rdump);
	}

	arch::opcode generator::encode_rload(const ast::instruction_statement& rload)
	{
		ensure_operands_count(rload, { 1 });

		if (make_operands_mask(rload) == arch::MASK_RLOAD_R8)
			return arch::_FX65(operand2reg(rload.operands[0]));

		throw generator_exception::invalid_operand_type(rload);
	}

	arch::opcode generator::encode_mov(const ast::instruction_statement& mov)
	{
		ensure_operands_count(mov, { 2 });

		switch (make_operands_mask(mov))
		{
			case arch::MASK_MOV_R8_R8: return arch::_8XY0(operand2reg(mov.operands[0]), operand2reg(mov.operands[1]));
			case arch::MASK_MOV_R8_I8: return arch::_6XNN(operand2reg(mov.operands[0]), operand2imm(mov.operands[1]));
			case arch::MASK_MOV_R8_DT: return arch::_FX07(operand2reg(mov.operands[0]));
			case arch::MASK_MOV_DT_R8: return arch::_FX15(operand2reg(mov.operands[1]));
			case arch::MASK_MOV_ST_R8: return arch::_FX18(operand2reg(mov.operands[1]));
			case arch::MASK_MOV_AR_R8: return arch::_FX29(operand2reg(mov.operands[1]));

			case arch::MASK_MOV_AR_I12:
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
		ensure_operands_count(draw, { 3 });

		if (make_operands_mask(draw) == arch::MASK_DRAW_R8_R8_I8)
			return arch::_DXYN(
					operand2reg(draw.operands[0]),
					operand2reg(draw.operands[1]),
					operand2imm(draw.operands[2], arch::fmt_imm4));

		throw generator_exception::invalid_operand_type(draw);
	}

	arch::opcode generator::encode_cls(const ast::instruction_statement& cls)
	{
		ensure_operands_count(cls, { 0 });

		if (!cls.operands.empty())
			throw generator_exception::invalid_operand_type(cls);

		return arch::_00E0();
	}

	arch::opcode generator::encode_rand(const ast::instruction_statement& rand)
	{
		ensure_operands_count(rand, { 2 });

		if (make_operands_mask(rand) == arch::MASK_RAND_R8_I8)
			return arch::_CXNN(
					operand2reg(rand.operands[0]),
					operand2imm(rand.operands[1]));

		throw generator_exception::invalid_operand_type(rand);
	}

	arch::opcode generator::encode_bcd(const ast::instruction_statement& bcd)
	{
		ensure_operands_count(bcd, { 1 });

		if (make_operands_mask(bcd) == arch::MASK_BCD_R8)
			return arch::_FX33(operand2reg(bcd.operands[0]));

		throw generator_exception::invalid_operand_type(bcd);
	}

	arch::opcode generator::encode_wkey(const ast::instruction_statement& wkey)
	{
		ensure_operands_count(wkey, { 1 });

		if (make_operands_mask(wkey) == arch::MASK_WKEY_R8)
			return arch::_FX0A(operand2reg(wkey.operands[0]));

		throw generator_exception::invalid_operand_type(wkey);
	}

	arch::opcode generator::encode_ske(const ast::instruction_statement& ske)
	{
		ensure_operands_count(ske, { 1 });

		if (make_operands_mask(ske) == arch::MASK_SKE_R8)
			return arch::_EX9E(operand2reg(ske.operands[0]));

		throw generator_exception::invalid_operand_type(ske);
	}

	arch::opcode generator::encode_skne(const ast::instruction_statement& skne)
	{
		ensure_operands_count(skne, { 1 });

		if (make_operands_mask(skne) == arch::MASK_SKNE_R8)
			return arch::_EXA1(operand2reg(skne.operands[0]));

		throw generator_exception::invalid_operand_type(skne);
	}

	arch::opcode generator::encode_ret(const ast::instruction_statement& ret)
	{
		ensure_operands_count(ret, { 0 });

		if (!ret.operands.empty())
			throw generator_exception::invalid_operand_type(ret);

		return arch::_00EE();
	}

	arch::opcode generator::encode_jmp(const ast::instruction_statement& jmp)
	{
		ensure_operands_count(jmp, { 1 });

		switch (make_operands_mask(jmp))
		{
			case arch::MASK_JMP_I12:
				if (jmp.operands[0].is_label())
				{
					// jmp @label
					register_patch_location(current_proc_name + "." + jmp.operands[0].operand.to_string());

					return arch::_1NNN(0);
				}

			// jmp [offset]
			case arch::MASK_JMP_INDIRECT_I12:
				return arch::_BNNN(operand2imm(jmp.operands[0], arch::fmt_imm12));

			default:
				throw generator_exception::invalid_operand_type(jmp);
		}
	}

	arch::opcode generator::encode_call(const ast::instruction_statement& call)
	{
		ensure_operands_count(call, { 1 });

		switch (make_operands_mask(call))
		{
			case arch::MASK_CALL_I12:
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
		ensure_operands_count(se, { 2 });

		switch (make_operands_mask(se))
		{
			case arch::MASK_SE_R8_R8:
				return arch::_5XY0(
						operand2reg(se.operands[0]),
						operand2reg(se.operands[1]));

			case arch::MASK_SE_R8_I8:
				return arch::_3XNN(
						operand2reg(se.operands[0]),
						operand2imm(se.operands[1]));

			default:
				throw generator_exception::invalid_operand_type(se);
		}
	}

	arch::opcode generator::encode_sne(const ast::instruction_statement& sne)
	{
		ensure_operands_count(sne, { 2 });

		switch (make_operands_mask(sne))
		{
			case arch::MASK_SNE_R8_R8:
				return arch::_9XY0(
						operand2reg(sne.operands[0]),
						operand2reg(sne.operands[1]));

			case arch::MASK_SNE_R8_I8:
				return arch::_4XNN(
						operand2reg(sne.operands[0]),
						operand2imm(sne.operands[1]));

			default:
				throw generator_exception::invalid_operand_type(sne);
		}
	}

	arch::opcode generator::encode_inc(const ast::instruction_statement& inc)
	{
		ensure_operands_count(inc, { 1 });

		if (make_operands_mask(inc) == arch::MASK_INC_R8)
			return arch::_7XNN(operand2reg(inc.operands[0]), 1);

		throw generator_exception::invalid_operand_type(inc);
	}

	std::vector<arch::opcode> generator::encode_swp(const ast::instruction_statement& swp)
	{
		ensure_operands_count(swp, { 2 });

		if (make_operands_mask(swp) == arch::MASK_SWP_R8_R8)
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