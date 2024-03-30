#include <ccomp/ast.hpp>
#include <ccomp/symbol_sanitizer.hpp>
#include <ccomp/generator.hpp>


namespace ccomp::ast
{
	abstract_tree::abstract_tree(std::vector<ast::statement> &&statements_list)
		: branches(std::move(statements_list))
	{}

	std::vector<uint16_t> abstract_tree::generate() const
	{
		sanitize();

		generator generator;

		return generator.generate(*this);
	}

	void abstract_tree::sanitize() const
	{
		symbol_sanitizer sanitizer;

		sanitizer.traverse(*this);
	}
}
