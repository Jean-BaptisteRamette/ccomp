#include <ccomp/instructions.hpp>
#include <ccomp/parser.hpp>
#include <ccomp/log.hpp>


namespace ccomp
{

#ifdef UNIT_TESTS_ON
    std::unique_ptr<parser> parser::from_buffer(std::string_view buff)
    {
        auto lexer = ccomp::lexer::from_buffer(buff);

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

    ast::abstract_tree parser::make_ast()
    {
        ast::abstract_tree ast;

		while (auto block = parse_next_block())
			ast.add_statement(std::move(block));

        return ast;
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
		expect(token_type::keyword);
		expect(token_type::special_character);

		auto token = expect(token_type::numerical, token_type::identifier);

		expect(token_type::special_character);

		return std::make_unique<ast::raw_statement>(std::move(token));
	}

	ast::statement parser::parse_define()
	{
		// consume "define" token
		expect(token_type::keyword);

		auto identifier = expect(token_type::identifier);
		auto value = expect(token_type::numerical);

		return std::make_unique<ast::define_statement>(
									std::move(identifier),
									std::move(value)
								);
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
