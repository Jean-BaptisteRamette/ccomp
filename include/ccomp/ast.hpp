#ifndef CCOMP_AST_HPP
#define CCOMP_AST_HPP


#include <string_view>
#include <memory>
#include <vector>

#include <ccomp/statements.hpp>


namespace ccomp::ast
{
    class abstract_tree
    {
    public:
        explicit abstract_tree(std::vector<ast::statement>&& statements_list);

		void sanitize() const;

        std::vector<ast::statement> branches {};
    };
}


#endif //CCOMP_AST_HPP
