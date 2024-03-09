#ifndef CCOMP_STATEMENTS_HPP
#define CCOMP_STATEMENTS_HPP

#include <ccomp/lexer.hpp>
#include <list>


namespace ccomp::ast
{
	struct base_statement;

    struct subroutine_statement;
    struct oper_node;
    struct instruction_statement;
    struct define_statement;
    struct raw_statement;

	using statement = std::unique_ptr<base_statement>;

    struct base_statement
    {
		base_statement() = default;
		base_statement(const base_statement&) = delete;
		base_statement(base_statement&&) = delete;
		base_statement& operator=(const base_statement&) = delete;
		base_statement& operator=(base_statement&&) = delete;

        virtual ~base_statement() = default;
    };

    struct subroutine_statement : base_statement
    {
        subroutine_statement(std::string name_, std::list<statement> inner_statements_)
            : name(std::move(name_)),
              inner_statements(std::move(inner_statements_))
        {}

        std::string name;
        std::list<statement> inner_statements; // statements and instructions
    };

    struct oper_node : base_statement
    {
        bool indirection {};

        // note: we use a token because we need to know
        // if the operand is number, identifier, or a register and if so which type
        ccomp::token token;
    };

    struct instruction_statement : base_statement
    {
        std::string_view mnemonic;
        char operands;

        // chip-8 instructions all have up to 2 operands
        // except the draw instruction which has 3
        oper_node lhs{};
        oper_node rhs{};
        oper_node opt{};
    };

    struct define_statement : base_statement
    {
        define_statement(token identifier_, token value_)
            : base_statement(),
			  identifier(std::move(identifier_)),
			  value(std::move(value_))
        {}

        token identifier;
        token value;
    };

    struct raw_statement : base_statement
    {
		explicit raw_statement(token opcode_)
			: base_statement(),
			  opcode(std::move(opcode_))
		{}

        token opcode;
    };
}

#endif //CCOMP_STATEMENTS_HPP
