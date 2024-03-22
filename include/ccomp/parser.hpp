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
	namespace parser_exception
	{
		struct parser_error : std::runtime_error
		{
			explicit parser_error(std::string_view message)
					: std::runtime_error(message.data()) {}
		};

		struct expected_more_error : parser_error
		{
			explicit expected_more_error(const token& last_token_)
					: parser_error(std::format("Expected more tokens after last token \"{}\" while parsing at line {} column {}",
											   ccomp::to_string(last_token_),
											   last_token_.source_location.line,
											   last_token_.source_location.col)),
					  last_token(last_token_)
			{}

			const token last_token;
		};

		struct expected_others_error : parser_error
		{
			expected_others_error(const token& unexpected_, token_type expected_type)
					: expected_others_error(unexpected_, { expected_type })
			{}

			expected_others_error(const token& unexpected_, std::initializer_list<token_type> expected_types_)
					: parser_error(std::format("Parser got token \"{}\" but expected a token of type {} while parsing at line {} column {}.",
											   ccomp::to_string(unexpected_),
											   ccomp::to_string(expected_types_),
											   unexpected_.source_location.line,
											   unexpected_.source_location.col)),
					  unexpected(unexpected_),
					  expected_types(expected_types_)
			{}

			const token unexpected;
			const std::initializer_list<token_type> expected_types;
		};

		struct unexpected_error : parser_error
		{
			explicit unexpected_error(const token& unexpected_)
					: parser_error(std::format("Unexpected token {} while parsing at line {} column {}.",
											   ccomp::to_string(unexpected_),
											   unexpected_.source_location.line,
											   unexpected_.source_location.col)),
					  unexpected(unexpected_)
			{}

			const token unexpected;
		};
	}


    class parser
    {
    public:
        explicit parser(std::vector<token>&& tokens_list);

#ifdef UNIT_TESTS_ON
        CCOMP_NODISCARD
        static std::unique_ptr<parser> from_buffer(std::string_view buff);
#endif

        ~parser() = default;

        CCOMP_NODISCARD ast::abstract_tree make_ast();

    CCOMP_PRIVATE:

		template<typename... Args>
		token expect(Args... types)
		{
			std::initializer_list<token_type> expected_types { types... };
			const token t = advance();

			for (auto expected : expected_types)
				if (t.type == expected)
					return t;

			throw parser_exception::expected_others_error(t, expected_types);
		};

		token advance();
        CCOMP_NODISCARD size_t remaining_tokens() const;

        CCOMP_NODISCARD ast::statement parse_primary_block();
        CCOMP_NODISCARD ast::statement parse_raw();
        CCOMP_NODISCARD ast::statement parse_define();
        CCOMP_NODISCARD ast::statement parse_instruction();
        CCOMP_NODISCARD ast::statement parse_subroutine();
		CCOMP_NODISCARD ast::statement parse_label();
        CCOMP_NODISCARD ast::statement parse_operand();

    CCOMP_PRIVATE:
        const std::vector<token> tokens;
        std::vector<token>::const_iterator token_it;
    };
}


#endif //CCOMP_PARSER_HPP
