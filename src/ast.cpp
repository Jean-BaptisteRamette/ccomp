#include <ccomp/ast.hpp>

namespace ccomp::ast
{
    void intermediate_representation::add_statement(ast::statement node)
    {
        branches.push_back(std::move(node));
    }
}
