#include <ccomp/ast.hpp>

namespace ccomp::ast
{
    void tree::add_branch(std::unique_ptr<ast::node> node)
    {
        branches.push_back(std::move(node));
    }
}
