#ifndef CCOMP_STATEMENT_NODE_HPP
#define CCOMP_STATEMENT_NODE_HPP

#include <ccomp/lexer.hpp>
#include <list>


namespace ccomp::ast
{
	struct statement_node;

    struct subr_node;
    struct oper_node;
    struct inst_node;
    struct defn_node;
    struct raw_node;

	using statement = std::unique_ptr<statement_node>;

    struct statement_node
    {
		statement_node() = default;
		statement_node(const statement_node&) = delete;
		statement_node(statement_node&&) = delete;
		statement_node& operator=(const statement_node&) = delete;
		statement_node& operator=(statement_node&&) = delete;

        virtual ~statement_node() = default;
    };

    struct subr_node : statement_node
    {
        subr_node(std::string name_, std::list<statement> nodes_)
            : name(std::move(name_)),
              nodes(std::move(nodes_))
              {}

        std::string name;
        std::list<statement> nodes; // statements and instructions
    };

    struct oper_node : statement_node
    {
        bool indirection {};

        // note: we use a token because we need to know
        // if the operand is number, identifier, or a register and if so which type
        ccomp::token token;
    };

    struct inst_node : statement_node
    {
        std::string_view mnemonic;
        char operands;

        // chip-8 instructions all have up to 2 operands
        // except the draw instruction which has 3
        oper_node lhs{};
        oper_node rhs{};
        oper_node opt{};
    };

    struct defn_node : statement_node
    {
        defn_node(token identifier_, token value_)
            : statement_node(),
			  identifier(std::move(identifier_)),
			  value(std::move(value_))
              {}

        token identifier;
        token value;
    };

    struct raw_node : statement_node
    {
        std::string opcode;
    };
}

#endif //CCOMP_STATEMENT_NODE_HPP
