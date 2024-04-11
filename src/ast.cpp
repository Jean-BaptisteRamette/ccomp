#include <chasm/ast.hpp>
#include <chasm/symbol_sanitizer.hpp>
#include <chasm/generator.hpp>


namespace chasm::ast
{
	abstract_tree::abstract_tree(std::vector<ast::statement> &&branches)
		: statements(std::move(branches))
	{}

	std::vector<uint16_t> abstract_tree::generate()
	{
		sanitize();

		std::ranges::stable_sort(statements, [](const ast::statement& a,
												const ast::statement& b)
		{
			return a->priority() > b->priority();
		});

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
