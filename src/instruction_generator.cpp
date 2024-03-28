#include <ccomp/instruction_generator.hpp>
#include <ccomp/instructions.hpp>


namespace ccomp
{
	std::vector<uint16_t> instruction_generator::generate(const ast::abstract_tree& ast)
	{
		for (const auto& branch : ast.branches)
			branch->accept(*this);

		post_visit();

		return binary;
	}

	void instruction_generator::post_visit()
	{
		// TODO: Apply call/jmp patches
	}

	void instruction_generator::visit(const ast::procedure_statement&)
	{
		// push the scope

		// visit inner

		// pop the scope
	}

	void instruction_generator::visit(const ast::instruction_statement& instruction)
	{
		if (!operands_types_ok(instruction))
			throw generator_exception::generator_invalid_operands_types();

		binary.push_back(0);
	}

	void instruction_generator::visit(const ast::define_statement&)
	{
		// store the value in a map
	}

	void instruction_generator::visit(const ast::raw_statement&)
	{
		// check if it encodes to a valid instruction
	}

	void instruction_generator::visit(const ast::label_statement&)
	{
		// push the scope

		// visit inner

		// pop the scope
	}

	bool instruction_generator::operands_types_ok(const ast::instruction_statement& instruction) const
	{

	}
}