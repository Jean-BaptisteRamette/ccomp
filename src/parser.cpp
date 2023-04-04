#include <ccomp/instructions.hpp>
#include <ccomp/parser.hpp>
#include <ccomp/log.hpp>


namespace ccomp
{

#ifdef UNIT_TESTS_ON
    std::unique_ptr<parser> parser::from_buff(std::string_view buff)
    {
        auto lexer = ccomp::lexer::from_buff(buff);

        std::vector<ccomp::token> tokens;

        for (auto token = lexer->next_token(); token.type != ccomp::token_type::eof; token = lexer->next_token())
            tokens.push_back(token);

        return std::make_unique<parser>(std::move(tokens));
    }
#endif


    parser::parser(std::vector<ccomp::token> &&tokens_list)
        : tokens(std::move(tokens_list)), token(std::begin(tokens))
    {}

    size_t parser::remaining_tokens() const
    {
        return std::distance(token, std::end(tokens));
    }

    ast::tree parser::make_tree()
    {

        ast::tree tree;

        for (auto block = expr_block(); block != nullptr; block = expr_block())
            tree.add_branch(std::move(block));

        return tree;
    }

    std::unique_ptr<ast::node> parser::expr_block()
    {
        if (!remaining_tokens())
            return nullptr;

        switch (token->type)
        {
            case token_type::keyword:
                if (token->lexeme == "define") return expr_define();

            // case token_type::special_character:
            //     if (token->lexeme == ".") return expr_subroutine();

            default:
                break;
        }

        throw parser_exception::unexpected_error(
                "block",
                token->lexeme
        );
    }

    std::unique_ptr<ast::subr_node> parser::expr_subroutine()
    {
       //
       //  ++token;
       //
       //  if (!remaining_tokens())
       //      throw parser_exception::expected_more_error("subroutine", tokens.back().lexeme);
       //
       //  std::string_view name = token->lexeme;
       //
       //  ++token;
       //  if (token->lexeme != ":")
       //      throw parser_exception::expected_else_error("subroutine", name, token->lexeme, ":");
       //
       //  ++token;
       //
       //
       //  return std::make_unique<ast::subr_node>();
       return nullptr;
    }

    std::unique_ptr<ast::defn_node> parser::expr_define()
    {
        ++token;

        if (remaining_tokens() < 2)
            throw parser_exception::expected_more_error("define", tokens.back().lexeme);

        std::string_view identifier = token->lexeme;

        ++token;

        std::string_view value = token->lexeme;

        ++token;

        return std::make_unique<ast::defn_node>(identifier, value);
    }

    std::unique_ptr<ast::raw_node> parser::expr_raw()
    {
        return nullptr;
    }

    std::unique_ptr<ast::inst_node> parser::expr_instruction()
    {
        /*
        ast::inst_node node = {
            .mnemonic = token->lexeme,
            .operands = inst::operands_count(token->lexeme)
        };

        if (node.operands <= 0)
            return node;

        ++token;

        // add rc, 0x10
        auto operands_left = node.operands;  // 2

        if (node.operands > 0) node.lhs = expr_operand(--operands_left);
        if (node.operands > 1) node.rhs = expr_operand(--operands_left);
        if (node.operands > 2) node.opt = expr_operand(--operands_left);

        return node;
         */

        return nullptr;
    }

    std::unique_ptr<ast::oper_node> parser::expr_operand(char operands_remain)
    {
         // auto is_reg_token = [&]() -> bool
         // {
         //     return token->type == token_type::gp_register || token->type == token_type::special_register;
         // };

         // if (remaining_tokens() < 1)
         //     throw parser_exception::expected_more_error("operands", tokens.back().lexeme);


        return nullptr;
    }
}















