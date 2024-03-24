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
				: std::runtime_error(message.data())
			{}

			template<typename ...Args>
			explicit parser_error(std::string_view fmt_message, Args&&... args)
				: std::runtime_error(std::vformat(fmt_message, std::make_format_args(args...)))
			{}
		};

		struct unmatching_procedure_names : parser_error
		{
			unmatching_procedure_names(const token& proc_name_beg, const token& proc_name_end)
				: parser_error(
					R"(Different procedure names at lines {} and {} ("{}" != "{}").)",
					proc_name_beg.source_location.line,
					proc_name_end.source_location.line,
					ccomp::to_string(proc_name_beg),
					ccomp::to_string(proc_name_end))
			{}
		};

		struct expected_others_error : parser_error
		{
			expected_others_error(const token& unexpected_, std::initializer_list<token_type> expected_types_)
				: parser_error(
					"Parser got token \"{}\" but expected a token of type {} while parsing at {}.",
					ccomp::to_string(unexpected_),
					ccomp::to_string(expected_types_),
					ccomp::to_string(unexpected_.source_location))
			{}
		};

		struct unexpected_error : parser_error
		{
			explicit unexpected_error(const token& unexpected_)
				: parser_error(
					"Unexpected token {} while parsing at {}.",
					ccomp::to_string(unexpected_),
					ccomp::to_string(unexpected_.source_location))
			{}
		};
	}


    class parser
    {
    public:
        explicit parser(std::vector<token>&& tokens_list);

        ~parser() = default;

        [[nodiscard]] ast::abstract_tree make_tree();

    private:
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

		[[nodiscard]] token advance();
        [[nodiscard]] bool no_more_tokens() const;

        [[nodiscard]] ast::statement parse_primary_statement();
        [[nodiscard]] ast::statement parse_raw();
        [[nodiscard]] ast::statement parse_define();
        [[nodiscard]] ast::statement parse_instruction();
        [[nodiscard]] ast::statement parse_procedure();
		[[nodiscard]] ast::statement parse_label();
        [[nodiscard]] ast::instruction_operand parse_operand();
		[[nodiscard]] std::vector<ast::instruction_operand> parse_operands(std::string_view mnemonic);

    private:
        const std::vector<token> tokens;
        std::vector<token>::const_iterator token_it;
    };
}


#endif //CCOMP_PARSER_HPP
