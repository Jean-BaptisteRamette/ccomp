#ifndef CCOMP_PARSER_HPP
#define CCOMP_PARSER_HPP


#include <unordered_map>
#include <string_view>
#include <vector>
#include <format>

#include <ccomp/lexer.hpp>
#include <ccomp/ast.hpp>


namespace ccomp
{
    class parser
    {
    public:
        explicit parser(std::vector<token>&& tokens_list);

#ifdef UNIT_TESTS_ON
        CCOMP_NODISCARD
        static std::unique_ptr<parser> from_buff(std::string_view buff);
#endif

        ~parser() = default;

        CCOMP_NODISCARD
        ast::tree make_tree();

    CCOMP_PRIVATE:
        CCOMP_NODISCARD size_t remaining_tokens() const;

        CCOMP_NODISCARD std::unique_ptr<ast::node>      expr_block();
        CCOMP_NODISCARD std::unique_ptr<ast::raw_node>  expr_raw();
        CCOMP_NODISCARD std::unique_ptr<ast::defn_node> expr_define();
        CCOMP_NODISCARD std::unique_ptr<ast::inst_node> expr_instruction();
        CCOMP_NODISCARD std::unique_ptr<ast::subr_node> expr_subroutine();
        CCOMP_NODISCARD std::unique_ptr<ast::oper_node> expr_operand(char operands_remain);

    CCOMP_PRIVATE:
        const std::vector<token> tokens;
        std::vector<token>::const_iterator token;
    };


    namespace parser_exception
    {
        struct parser_error : std::runtime_error
        {
            explicit parser_error(std::string_view parsing_context_, std::string_view message)
                : parsing_context(parsing_context_), std::runtime_error(message.data()) {}

            // TODO: more precise parsing context
            std::string_view parsing_context;
        };

        struct expected_more_error : parser_error
        {
            explicit expected_more_error(std::string_view parsing_context, std::string_view last_lexeme_)
                : parser_error(parsing_context, std::format("Expected more tokens to parse after last token {} while parsing {}", last_lexeme, parsing_context)),
                  last_lexeme(last_lexeme_)
                  {}

            std::string_view last_lexeme;
        };

        struct expected_else_error: parser_error
        {
            explicit expected_else_error(std::string_view parsing_context, std::string_view last_lexeme_, std::string_view unexpected_, std::string_view expected_)
                : parser_error(parsing_context, std::format(R"(Expected token among "{}" after last token {} while parsing {} but got "{}")", expected_, last_lexeme_, parsing_context, unexpected_)),
                  last_lexeme(last_lexeme_),
                  unexpected(unexpected_),
                  expected(expected_)
                  {}

            std::string_view last_lexeme;
            std::string_view unexpected;
            std::string_view expected;
        };

        struct unexpected_error : parser_error
        {
            explicit unexpected_error(std::string_view parsing_context, std::string_view unexpected_)
                : parser_error(parsing_context, std::format(R"(Unexpected token {} while parsing {})", unexpected_, parsing_context)),
                  unexpected(unexpected_)
                  {}

                  std::string_view unexpected;
        };
    }
}


#endif //CCOMP_PARSER_HPP
