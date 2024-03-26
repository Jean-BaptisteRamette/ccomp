#include <ccomp/ast.hpp>
#include <ccomp/symbol_sanitizer.hpp>


namespace ccomp::ast
{
	abstract_tree::abstract_tree(std::vector<ast::statement> &&statements_list)
		: branches(std::move(statements_list))
	{}

	void abstract_tree::sanitize() const
	{
		symbol_sanitizer sanitizer;

		sanitizer.traverse(*this);
	}
}
