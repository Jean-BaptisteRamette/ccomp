#ifndef CCOMP_PARSER_HPP
#define CCOMP_PARSER_HPP


#include <unordered_set>
#include <unordered_map>
#include <string_view>
#include <vector>
#include <format>

#include <ccomp/assembler_error.hpp>
#include <ccomp/lexer.hpp>
#include <ccomp/ast.hpp>


namespace ccomp
{
	namespace parser_exception
	{
		struct unmatching_procedure_names : assembler_error
		{
			unmatching_procedure_names(const token& proc_name_beg, const token& proc_name_end)
				: assembler_error(
					R"(Different procedure names at lines {} and {} ("{}" != "{}").)",
					proc_name_beg.source_location.line,
					proc_name_end.source_location.line,
					proc_name_beg.to_string(),
					proc_name_end.to_string())
			{}
		};

		struct expected_others_error : assembler_error
		{
			expected_others_error(const token& unexpected_, std::initializer_list<token_type> expected_types_)
				: assembler_error(
					"Parser got token \"{}\" but expected a token of type {} while parsing at {}.",
					unexpected_.to_string(),
					ccomp::to_string(expected_types_),
					ccomp::to_string(unexpected_.source_location))
			{}
		};

		struct unexpected_error : assembler_error
		{
			explicit unexpected_error(const token& unexpected_)
				: assembler_error(
					"Unexpected token {} while parsing at {}.",
					unexpected_.to_string(),
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

		template<typename... Args>
		[[nodiscard]] bool advance_if(Args... types)
		{
			std::unordered_set<token_type> expected_types { types... };

			if (expected_types.contains(token_it->type))
			{
				advance();
				return true;
			}

			return false;
		};

		template<typename... Args>
		[[nodiscard]] bool next_any_of(Args... types)
		{
			if (no_more_tokens())
				return false;

			std::unordered_set<token_type> set { types... };

			return set.contains(token_it->type);
		};

		token advance();
        [[nodiscard]] bool no_more_tokens() const;

        [[nodiscard]] ast::statement parse_primary_statement();
        [[nodiscard]] ast::statement parse_raw();
        [[nodiscard]] ast::statement parse_define();
		[[nodiscard]] ast::statement parse_sprite();
        [[nodiscard]] ast::statement parse_instruction();
        [[nodiscard]] ast::statement parse_procedure();
		[[nodiscard]] ast::statement parse_label();
        [[nodiscard]] ast::instruction_operand parse_operand();
		[[nodiscard]] std::vector<ast::instruction_operand> parse_operands();

    private:
        const std::vector<token> tokens;
        std::vector<token>::const_iterator token_it;
    };
}


#endif //CCOMP_PARSER_HPP
