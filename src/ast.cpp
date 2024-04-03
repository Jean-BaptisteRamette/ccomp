#include <ccomp/ast.hpp>
#include <ccomp/symbol_sanitizer.hpp>
#include <ccomp/generator.hpp>


namespace ccomp::ast
{
	abstract_tree::abstract_tree(std::vector<ast::statement> &&branches)
		: statements(std::move(branches))
	{
		// TODO: We may need to have this after symbol sanitizing ?
		// std::ranges::stable_sort(statements, [](const ast::statement& a,
		// 										const ast::statement& b)
		// {
		// 	return a->priority() > b->priority();
		// });
	}

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

	const std::vector<ast::statement>& abstract_tree::branches() const
	{
		return statements;
	}
}
