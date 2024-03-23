#ifndef CCOMP_AST_HPP
#define CCOMP_AST_HPP


#include <string_view>
#include <memory>
#include <vector>

#include <ccomp/statements.hpp>


namespace ccomp::ast
{
    // define numa 1
    //
    // .main:
    //     define numb 2
    //     add r1, numa
    //     add r2, numb
    //
    // the root node is just a node containing a list of other nodes aka blocks.
    // the list of blocks:  <define node> <subroutine node>
    //
    // define node contain an identifier and a value
    // subroutine node contains a list of nodes.
    // the list is: <define node> <instruction node> <instruction node>
    //
    // instruction nodes contain operands and a mnemonic
    //
    //
    //
    class abstract_tree
    {
    public:
        explicit abstract_tree(std::vector<ast::statement>&& statements_list);

    CCOMP_PRIVATE:
        std::vector<ast::statement> branches {};
    };
}


#endif //CCOMP_AST_HPP
