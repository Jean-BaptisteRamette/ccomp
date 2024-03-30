#include <ccomp/generator.hpp>
#include <ccomp/arch.hpp>


namespace ccomp
{
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

			mask |= (m << shift);
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
		binary.push_back(to_integer(statement.opcode));
	}

	void generator::visit(const ast::label_statement&)
	{

	}

	uint16_t generator::to_integer(const token& token) const
	{
		if (token.type == token_type::numerical)
			return token.to_integer();

		return constants.at(token.to_string());
	}

	arch::opcode generator::encode_add(const std::vector<ast::instruction_operand>& operands) const
	{
		switch (make_operands_mask(operands))
		{
			case arch::MASK_ADD_R8_R8:
				return arch::_8XY4(
						arch::regname2regindex(operands[0].operand.to_string()),
						arch::regname2regindex(operands[1].operand.to_string()));

			case arch::MASK_ADD_R8_I8:
				return arch::_7XNN(
						arch::regname2regindex(operands[0].operand.to_string()),
						to_integer(operands[1].operand));

			default:
				throw generator_exception::generator_invalid_operand_type();
		}
	}

	arch::opcode generator::encode_sub(const std::vector<ast::instruction_operand>& operands) const
	{
		if (make_operands_mask(operands) == arch::MASK_SUB_R8_R8)
			return arch::_8XY5(
					arch::regname2regindex(operands[0].operand.to_string()),
					arch::regname2regindex(operands[1].operand.to_string()));

		throw generator_exception::generator_invalid_operand_type();
	}
}