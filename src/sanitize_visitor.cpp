#include <ccomp/sanitize_visitor.hpp>
#include <iostream>


namespace ccomp::ast
{
	void sanitize_visitor::visit(const procedure_statement&)
	{
		std::cout << "procedure_statement" << std::endl;
	}

	void sanitize_visitor::visit(const instruction_statement&)
	{
		std::cout << "instruction_statement" << std::endl;
	}

	void sanitize_visitor::visit(const define_statement&)
	{
		std::cout << "define_statement" << std::endl;
	}

	void sanitize_visitor::visit(const raw_statement&)
	{
		std::cout << "raw_statement" << std::endl;
	}

	void sanitize_visitor::visit(const label_statement&)
	{
		std::cout << "label_statement" << std::endl;
	}
}
