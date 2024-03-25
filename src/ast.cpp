#include <ccomp/ast.hpp>
#include <ccomp/sanitize_visitor.hpp>


namespace ccomp::ast
{
	abstract_tree::abstract_tree(std::vector<ast::statement> &&statements_list)
		: branches(std::move(statements_list))
	{}

	void abstract_tree::sanitize() const
	{
		sanitize_visitor sanitizer;

		sanitizer.traverse(*this);
	}
}
