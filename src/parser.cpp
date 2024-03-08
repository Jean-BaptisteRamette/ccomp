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


    parser::parser(std::vector<token> &&tokens_list)
        : tokens(std::move(tokens_list)),
		  token_it(std::begin(tokens))
    {}

	token parser::expect(token_type expected_type)
	{
		const token t = advance();

		if (t.type != expected_type)
			throw parser_exception::expected_other_error(t, expected_type);

		return t;
	}

	token parser::advance()
	{
		const token t = *token_it;
		++token_it;

		return t;
	}

    size_t parser::remaining_tokens() const
    {
        return std::distance(token_it, std::end(tokens));
    }

    ast::intermediate_representation parser::make_ir()
    {
        ast::intermediate_representation ir;

        for (auto block = parse_next_block(); block != nullptr; block = parse_next_block())
			ir.add_statement(std::move(block));

        return ir;
    }

	ast::statement parser::parse_next_block()
	{
		if (token_it == std::end(tokens))
			return {};

		if (token_it->type == token_type::keyword || token_it->type == token_type::special_character)
		{
			if (token_it->lexeme == "raw")
				return parse_raw();
			else if (token_it->lexeme == "define")
				return parse_define();
			else if (token_it->lexeme == ".")
				return parse_subroutine();
		}

		throw parser_exception::unexpected_error(*token_it);
	}

	ast::statement parser::parse_raw()
	{
		return {};
	}

	ast::statement parser::parse_define()
	{
		expect(token_type::keyword);

		const token identifier = expect(token_type::identifier);
		const token value = expect(token_type::numerical);

		auto statement = std::make_unique<ast::defn_node>(identifier, value);

		return statement;
	}

	ast::statement parser::parse_instruction()
	{
		return {};
	}

	ast::statement parser::parse_subroutine()
	{
		return {};
	}

	ast::statement parser::parse_operand()
	{
		return {};
	}
}
