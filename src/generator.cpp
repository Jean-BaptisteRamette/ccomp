#include <span>
#include <ccomp/generator.hpp>
#include <ccomp/arch.hpp>


namespace ccomp
{
	arch::reg operand2reg(const ast::instruction_operand& operand)
	{
		return operand.reg_name()[1] - '0';
	}

	uint16_t make_operands_mask(const std::vector<ast::instruction_operand>& operands)
	{
		uint16_t mask = 0;
		uint16_t shift = 0;

		for (const auto& operand : operands)
		{
			mask |= (static_cast<uint8_t>(operand.arch_type()) << shift);
			shift += 3;
		}

		return mask;
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
		for (const auto& [addr, sym] : patches)
			binary[addr / sizeof(arch::opcode)] |= sym_addresses[sym];
	}

	void generator::visit(const ast::procedure_statement& procedure)
	{
		register_symbol_addr(procedure.name_beg.to_string());
	}

	void generator::visit(const ast::instruction_statement& instruction)
	{
		if (instruction.operands.size() > arch::MAX_OPERANDS)
			throw generator_exception::too_many_operands(instruction);

		const auto mnemonic = instruction.mnemonic.to_string();
		auto encoder = mnemonic_encoders.at(mnemonic);

		binary.push_back((this->*encoder)(instruction.operands));
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
		register_symbol_addr(label.identifier.to_string());
	}

	void generator::visit(const ast::raw_statement& statement)
	{
		binary.push_back(operand2imm(statement.opcode));
	}

	void generator::register_constant(std::string &&symbol, arch::imm value)
	{
		if (constants.contains(symbol))
			throw assembler_error("Generator found an already defined constant \"{}\", this should have been caught by the sanitizer.", symbol);

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

	void generator::register_patch_addr(std::string&& symbol)
	{
		patches.push_back({
			.addr = static_cast<arch::addr>(binary.size()),
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

	arch::opcode generator::encode_add(const std::vector<ast::instruction_operand>& operands)
	{
		switch (make_operands_mask(operands))
		{
			case arch::MASK_ADD_R8_R8:
				return arch::_8XY4(
						operand2reg(operands[0]),
						operand2reg(operands[1]));

			case arch::MASK_ADD_R8_I8:
				return arch::_7XNN(
						operand2reg(operands[0]),
						operand2imm(operands[1]));

			case arch::MASK_ADD_AR_R8:
				return arch::_FX1E(operand2reg(operands[0]));

			default:
				throw generator_exception::invalid_operand_type();
		}
	}

	arch::opcode generator::encode_sub(const std::vector<ast::instruction_operand>& operands)
	{
		if (make_operands_mask(operands) == arch::MASK_SUB_R8_R8)
			return arch::_8XY5(
					operand2reg(operands[0]),
					operand2reg(operands[1]));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_suba(const std::vector<ast::instruction_operand>& operands)
	{
		if (make_operands_mask(operands) == arch::MASK_SUBA_R8_R8)
			return arch::_8XY7(
					operand2reg(operands[0]),
					operand2reg(operands[1]));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_or(const std::vector<ast::instruction_operand>& operands)
	{
		if (make_operands_mask(operands) == arch::MASK_OR_R8_R8)
			return arch::_8XY1(
					operand2reg(operands[0]),
					operand2reg(operands[1]));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_and(const std::vector<ast::instruction_operand>& operands)
	{
		if (make_operands_mask(operands) == arch::MASK_AND_R8_R8)
			return arch::_8XY2(
					operand2reg(operands[0]),
					operand2reg(operands[1]));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_xor(const std::vector<ast::instruction_operand>& operands)
	{
		if (make_operands_mask(operands) == arch::MASK_XOR_R8_R8)
			return arch::_8XY3(
					operand2reg(operands[0]),
					operand2reg(operands[1]));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_shr(const std::vector<ast::instruction_operand>& operands)
	{
		switch (make_operands_mask(operands))
		{
			case arch::MASK_SHR_R8:
				return arch::_8X06(operand2reg(operands[0]));

			case arch::MASK_SHR_R8_R8:
				return arch::_8XY6(
						operand2reg(operands[0]),
						operand2reg(operands[1]));

			default:
				throw generator_exception::invalid_operand_type();
		}
	}

	arch::opcode generator::encode_shl(const std::vector<ast::instruction_operand>& operands)
	{
		switch (make_operands_mask(operands))
		{
			case arch::MASK_SHL_R8:
				return arch::_8X0E(operand2reg(operands[0]));

			case arch::MASK_SHL_R8_R8:
				return arch::_8XYE(
						operand2reg(operands[0]),
						operand2reg(operands[1]));

			default:
				throw generator_exception::invalid_operand_type();
		}
	}

	arch::opcode generator::encode_rdump(const std::vector<ast::instruction_operand>& operands)
	{
		if (make_operands_mask(operands) == arch::MASK_RDUMP_R8)
			return arch::_FX55(operand2reg(operands[0]));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_rload(const std::vector<ast::instruction_operand>& operands)
	{
		if (make_operands_mask(operands) == arch::MASK_RLOAD_R8)
			return arch::_FX65(operand2reg(operands[0]));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_mov(const std::vector<ast::instruction_operand>& operands)
	{
		switch (make_operands_mask(operands))
		{
			case arch::MASK_MOV_R8_R8: return arch::_8XY0(operand2reg(operands[0]), operand2reg(operands[1]));
			case arch::MASK_MOV_R8_I8: return arch::_6XNN(operand2reg(operands[0]), operand2imm(operands[1]));
			case arch::MASK_MOV_R8_DT: return arch::_FX07(operand2reg(operands[0]));
			case arch::MASK_MOV_DT_R8: return arch::_FX15(operand2reg(operands[0]));
			case arch::MASK_MOV_ST_R8: return arch::_FX18(operand2reg(operands[0]));
			case arch::MASK_MOV_AR_R8: return arch::_FX29(operand2reg(operands[0]));

			case arch::MASK_MOV_AR_I12:
			{
				if (operands[1].is_sprite())
				{
					register_patch_addr(operands[1].operand.to_string());
					return arch::_ANNN(0);
				}
				else
					return arch::_ANNN(operand2imm(operands[1], arch::imm12));
			}

			default:
				throw generator_exception::invalid_operand_type();
		}
	}

	arch::opcode generator::encode_swp(const std::vector<ast::instruction_operand>& operands)
	{
		// TODO
		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_draw(const std::vector<ast::instruction_operand>& operands)
	{
		if (make_operands_mask(operands) == arch::MASK_DRAW_R8_R8_I8)
			return arch::_DXYN(
					operand2reg(operands[0]),
					operand2reg(operands[1]),
					operand2imm(operands[2], arch::imm4));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_cls(const std::vector<ast::instruction_operand>& operands)
	{
		if (!operands.empty())
			throw generator_exception::invalid_operand_type();

		return arch::_00E0();
	}

	arch::opcode generator::encode_rand(const std::vector<ast::instruction_operand>& operands)
	{
		if (make_operands_mask(operands) == arch::MASK_RAND_R8_I8)
			return arch::_CXNN(
					operand2reg(operands[0]),
					operand2imm(operands[1]));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_bcd(const std::vector<ast::instruction_operand>& operands)
	{
		if (make_operands_mask(operands) == arch::MASK_BCD_R8)
			return arch::_FX33(operand2reg(operands[0]));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_wkey(const std::vector<ast::instruction_operand>& operands)
	{
		if (make_operands_mask(operands) == arch::MASK_WKEY_R8)
			return arch::_FX0A(operand2reg(operands[0]));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_ske(const std::vector<ast::instruction_operand>& operands)
	{
		if (make_operands_mask(operands) == arch::MASK_SKE_R8)
			return arch::_EX9E(operand2reg(operands[0]));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_skne(const std::vector<ast::instruction_operand>& operands)
	{
		if (make_operands_mask(operands) == arch::MASK_SKNE_R8)
			return arch::_EXA1(operand2reg(operands[0]));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_ret(const std::vector<ast::instruction_operand>& operands)
	{
		if (!operands.empty())
			throw generator_exception::invalid_operand_type();

		return arch::_00EE();
	}

	arch::opcode generator::encode_jmp(const std::vector<ast::instruction_operand>& operands)
	{
		switch (make_operands_mask(operands))
		{
			case arch::MASK_JMP_I12:
				if (operands[0].is_label())
				{
					// jmp @label
					register_patch_addr(operands[0].operand.to_string());

					return arch::_1NNN(0);
				}
				else
					// define where 0xBEEF
					// jmp where
					// jmp 0xBEEF
					return arch::_1NNN(operand2imm(operands[0], arch::imm12));

			// jmp [offset]
			case arch::MASK_JMP_INDIRECT_I12:
				return arch::_BNNN(operand2imm(operands[0], arch::imm12));

			default:
				throw generator_exception::invalid_operand_type();
		}
	}

	arch::opcode generator::encode_call(const std::vector<ast::instruction_operand>& operands)
	{
		switch (make_operands_mask(operands))
		{
			case arch::MASK_CALL_I12:
				if (operands[0].is_procedure())
				{
					// call $function
					register_patch_addr(operands[0].operand.to_string());

					return arch::_2NNN(0);
				}
				else
					// define my_addr 0xBEEF
					// call my_addr
					// call 0xBEEF
					return arch::_2NNN(operand2imm(operands[0], arch::imm12));

			default:
				throw generator_exception::invalid_operand_type();
		}
	}

	arch::opcode generator::encode_se(const std::vector<ast::instruction_operand>& operands)
	{
		switch (make_operands_mask(operands))
		{
			case arch::MASK_SE_R8_R8:
				return arch::_5XY0(
						operand2reg(operands[0]),
						operand2reg(operands[1]));

			case arch::MASK_SE_R8_I8:
				return arch::_3XNN(
						operand2reg(operands[0]),
						operand2imm(operands[1]));

			default:
				throw generator_exception::invalid_operand_type();
		}

	}

	arch::opcode generator::encode_sne(const std::vector<ast::instruction_operand>& operands)
	{
		switch (make_operands_mask(operands))
		{
			case arch::MASK_SNE_R8_R8:
				return arch::_9XY0(
						operand2reg(operands[0]),
						operand2reg(operands[1]));

			case arch::MASK_SNE_R8_I8:
				return arch::_4XNN(
						operand2reg(operands[0]),
						operand2imm(operands[1]));

			default:
				throw generator_exception::invalid_operand_type();
		}
	}

	arch::opcode generator::encode_inc(const std::vector<ast::instruction_operand>& operands)
	{
		if (make_operands_mask(operands) == arch::MASK_INC_R8)
			return arch::_7XNN(operand2reg(operands[0]), 1);

		throw generator_exception::invalid_operand_type();
	}
}