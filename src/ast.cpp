#include <ccomp/ast.hpp>

namespace ccomp::ast
{
    void abstract_tree::add_statement(ast::statement node)
    {
        branches.push_back(std::move(node));
    }
}
