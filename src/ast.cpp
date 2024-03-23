#include <ccomp/ast.hpp>

namespace ccomp::ast
{
	abstract_tree::abstract_tree(std::vector<ast::statement> &&statements_list)
		: branches(std::move(statements_list))
	{}
}
