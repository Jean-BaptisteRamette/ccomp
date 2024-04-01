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
		auto regname2optype = [](const auto& name) -> arch::operand_type
		{
			if (name == "ar") return arch::operand_type::reg_ar;
			if (name == "st") return arch::operand_type::reg_st;
			if (name == "dt") return arch::operand_type::reg_dt;

			return arch::operand_type::reg_rx;
		};

		uint16_t mask = 0;
		uint16_t shift = 0;

		for (const auto& operand : operands)
		{
			arch::operand_type m;

			if (operand.is_reg())
				m = regname2optype(operand.operand.to_string());
			else if (operand.is_imm())
				m = arch::operand_type::imm8;

			mask |= (static_cast<uint8_t>(m) << shift);
			shift += 3;
		}

		return mask;
	}

	std::vector<arch::opcode> generator::generate(const ast::abstract_tree& ast)
	{
		for (const auto& branch : ast.branches)
			branch->accept(*this);

		post_visit();

		return binary;
	}

	void generator::post_visit()
	{
		// TODO: Apply call/jmp patches
	}

	void generator::visit(const ast::procedure_statement&)
	{
		// visit inner
	}

	void generator::visit(const ast::instruction_statement& instruction)
	{
		const auto mnemonic = instruction.mnemonic.to_string();
		auto enc = mnemonic_encoders.at(mnemonic);

		// We don't want to push back directly for procedures
		// binary.push_back(encoder(instruction));
	}

	void generator::visit(const ast::define_statement& statement)
	{
		const auto sym = statement.identifier.to_string();
		const auto val = statement.value.to_integer();

		constants[sym] = val;
	}

	void generator::visit(const ast::sprite_statement&)
	{

	}

	void generator::visit(const ast::raw_statement& statement)
	{
		binary.push_back(operand2imm(statement.opcode));
	}

	void generator::visit(const ast::label_statement&)
	{

	}

	arch::imm generator::operand2imm(const token& token, arch::imm_format imm_max) const
	{
		arch::imm imm;

		if (token.type == token_type::numerical)
			imm = token.to_integer();
		else
		 	imm = constants.at(token.to_string());

		if (imm > imm_max)
			throw generator_exception::invalid_immediate_format();

		return imm;
	}

	arch::imm generator::operand2imm(const ast::instruction_operand& operand, arch::imm_format imm) const
	{
		return operand2imm(operand.operand, imm);
	}

	arch::opcode generator::encode_add(const std::vector<ast::instruction_operand>& operands) const
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

	arch::opcode generator::encode_sub(const std::vector<ast::instruction_operand>& operands) const
	{
		if (make_operands_mask(operands) == arch::MASK_SUB_R8_R8)
			return arch::_8XY5(
					operand2reg(operands[0]),
					operand2reg(operands[1]));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_suba(const std::vector<ast::instruction_operand>& operands) const
	{
		if (make_operands_mask(operands) == arch::MASK_SUBA_R8_R8)
			return arch::_8XY7(
					operand2reg(operands[0]),
					operand2reg(operands[1]));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_or(const std::vector<ast::instruction_operand>& operands) const
	{
		if (make_operands_mask(operands) == arch::MASK_OR_R8_R8)
			return arch::_8XY1(
					operand2reg(operands[0]),
					operand2reg(operands[1]));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_and(const std::vector<ast::instruction_operand>& operands) const
	{
		if (make_operands_mask(operands) == arch::MASK_AND_R8_R8)
			return arch::_8XY2(
					operand2reg(operands[0]),
					operand2reg(operands[1]));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_xor(const std::vector<ast::instruction_operand>& operands) const
	{
		if (make_operands_mask(operands) == arch::MASK_XOR_R8_R8)
			return arch::_8XY3(
					operand2reg(operands[0]),
					operand2reg(operands[1]));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_shr(const std::vector<ast::instruction_operand>& operands) const
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

	arch::opcode generator::encode_shl(const std::vector<ast::instruction_operand>& operands) const
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

	arch::opcode generator::encode_rdump(const std::vector<ast::instruction_operand>& operands) const
	{
		if (make_operands_mask(operands) == arch::MASK_RDUMP_R8)
			return arch::_FX55(operand2reg(operands[0]));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_rload(const std::vector<ast::instruction_operand>& operands) const
	{
		if (make_operands_mask(operands) == arch::MASK_RLOAD_R8)
			return arch::_FX65(operand2reg(operands[0]));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_mov(const std::vector<ast::instruction_operand>& operands) const
	{
		switch (make_operands_mask(operands))
		{
			case arch::MASK_MOV_R8_R8: return arch::_8XY0(operand2reg(operands[0]), operand2reg(operands[1]));
			case arch::MASK_MOV_R8_I8: return arch::_6XNN(operand2reg(operands[0]), operand2imm(operands[1]));
			case arch::MASK_MOV_R8_DT: return arch::_FX07(operand2reg(operands[0]));
			case arch::MASK_MOV_AR_R8: return arch::_FX29(operand2reg(operands[0]));
			case arch::MASK_MOV_DT_R8: return arch::_FX15(operand2reg(operands[0]));
			case arch::MASK_MOV_ST_R8: return arch::_FX18(operand2reg(operands[0]));

			default:
				throw generator_exception::invalid_operand_type();
		}
	}

	arch::opcode generator::encode_swp(const std::vector<ast::instruction_operand>& operands) const
	{
		// TODO
		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_draw(const std::vector<ast::instruction_operand>& operands) const
	{
		if (make_operands_mask(operands) == arch::MASK_DRAW_R8_R8_I8)
			return arch::_DXYN(
					operand2reg(operands[0]),
					operand2reg(operands[1]),
					operand2imm(operands[2], arch::imm4));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_cls(const std::vector<ast::instruction_operand>& operands) const
	{
		if (!operands.empty())
			throw generator_exception::invalid_operand_type();

		return arch::_00E0();
	}

	arch::opcode generator::encode_rand(const std::vector<ast::instruction_operand>& operands) const
	{
		if (make_operands_mask(operands) == arch::MASK_RAND_R8_I8)
			return arch::_CXNN(
					operand2reg(operands[0]),
					operand2imm(operands[1]));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_bcd(const std::vector<ast::instruction_operand>& operands) const
	{
		if (make_operands_mask(operands) == arch::MASK_BCD_R8)
			return arch::_FX33(operand2reg(operands[0]));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_wkey(const std::vector<ast::instruction_operand>& operands) const
	{
		if (make_operands_mask(operands) == arch::MASK_WKEY_R8)
			return arch::_FX0A(operand2reg(operands[0]));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_ske(const std::vector<ast::instruction_operand>& operands) const
	{
		if (make_operands_mask(operands) == arch::MASK_SKE_R8)
			return arch::_EX9E(operand2reg(operands[0]));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_skne(const std::vector<ast::instruction_operand>& operands) const
	{
		if (make_operands_mask(operands) == arch::MASK_SKNE_R8)
			return arch::_EXA1(operand2reg(operands[0]));

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_ret(const std::vector<ast::instruction_operand>& operands) const
	{
		if (!operands.empty())
			throw generator_exception::invalid_operand_type();

		return arch::_00EE();
	}

	arch::opcode generator::encode_jmp(const std::vector<ast::instruction_operand>& operands) const
	{
		// TODO

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_call(const std::vector<ast::instruction_operand>& operands) const
	{
		// TODO

		throw generator_exception::invalid_operand_type();
	}

	arch::opcode generator::encode_se(const std::vector<ast::instruction_operand>& operands) const
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

	arch::opcode generator::encode_sne(const std::vector<ast::instruction_operand>& operands) const
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

	arch::opcode generator::encode_inc(const std::vector<ast::instruction_operand>& operands) const
	{
		if (make_operands_mask(operands) == arch::MASK_INC_R8)
			return arch::_7XNN(operand2reg(operands[0]), 1);

		throw generator_exception::invalid_operand_type();
	}
}