#ifndef CCOMP_NODE_HPP
#define CCOMP_NODE_HPP

#include <ccomp/lexer.hpp>


namespace ccomp::ast
{
    struct subr_node;
    struct oper_node;
    struct inst_node;
    struct defn_node;
    struct raw_node;

    struct node_visitor
    {
        void visit_subr(const subr_node&);
        void visit_oper(const oper_node&);
        void visit_inst(const inst_node&);
        void visit_defn(const defn_node&);
        void visit_raw(const raw_node&);
    };

    enum class node_type
    {
        subr,
        oper,
        inst,
        defn,
        raw
    };

    struct node
    {
        node_type type;

        virtual ~node() = default;
        virtual void accept(node_visitor& visitor) = 0;
    };

    struct subr_node : node
    {
        subr_node(std::string_view name_, std::list<std::unique_ptr<node>> nodes_)
            : name(name_),
              nodes(std::move(nodes_))
              {}

        void accept(node_visitor& visitor) override { visitor.visit_subr(*this); }

        std::string_view name;
        std::list<std::unique_ptr<node>> nodes; // statements and instructions
    };

    struct oper_node : node
    {
        bool indirection {};

        // note: we use a token because we need to know
        // if the operand is number, identifier, or a register and if so which type
        ccomp::token token;

        void accept(node_visitor& visitor) override { visitor.visit_oper(*this); }
    };

    struct inst_node : node
    {
        std::string_view mnemonic;
        char operands;

        // chip-8 instructions all have up to 2 operands
        // except the draw instruction which has 3
        oper_node lhs{};
        oper_node rhs{};
        oper_node opt{};

        void accept(node_visitor& visitor) override { visitor.visit_inst(*this); }
    };

    struct defn_node : node
    {
        defn_node(std::string_view identifier_, std::string_view value_)
            : node(),
              identifier(identifier_),
              value(value_)
              {}

        void accept(node_visitor& visitor) override { visitor.visit_defn(*this); }

        std::string_view identifier;
        std::string_view value;
    };

    struct raw_node : node
    {
        std::string_view opcode;

        void accept(node_visitor& visitor) override { visitor.visit_raw(*this); }
    };
}

#endif //CCOMP_NODE_HPP
