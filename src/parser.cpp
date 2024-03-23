#include <ccomp/instructions.hpp>
#include <ccomp/parser.hpp>
#include <ccomp/log.hpp>


namespace ccomp
{
	namespace
	{
		const std::unordered_map<std::string_view, unsigned char> operands_count = {
						{ "add",   2 },
						{ "sub",   2 },
						{ "suba",  2 },
						{ "or",    2 },
						{ "and",   2 },
						{ "xor",   2 },
						{ "shr",   1 },
						{ "shl",   1 },
						{ "rdump", 1 },
						{ "rload", 1 },
						{ "mov",   2 },
						{ "swp",   2 },
						{ "draw",  3 },
						{ "cls",   0 },
						{ "rand",  2 },
						{ "bcd",   1 },
						{ "wkey",  1 },
						{ "ske",   1 },
						{ "snke",  1 },
						{ "ret",   0 },
						{ "jmp",   1 },
						{ "call",  1 },
		};
	}


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
		if (reached_eof())
			throw std::runtime_error("Unexpected EOF while parsing !");

		const token t = *token_it;
		++token_it;

		return t;
	}

    bool parser::reached_eof() const
    {
        return token_it == std::end(tokens);
    }

    ast::abstract_tree parser::make_tree()
    {
		std::vector<ast::statement> statements;

		while (auto block = parse_primary_statement())
			statements.push_back(std::move(block));

        return ast::abstract_tree(std::move(statements));
    }

	ast::statement parser::parse_primary_statement()
	{
		if (reached_eof())
			return {};

		switch (token_it->type)
		{
			case token_type::keyword_define:     return parse_define();
			case token_type::keyword_raw:        return parse_raw();
			case token_type::dot:                return parse_label();
			case token_type::keyword_proc_start: return parse_procedure();
			case token_type::instruction:        return parse_instruction();

			default:
				throw parser_exception::unexpected_error(*token_it);
		}
	}

	ast::statement parser::parse_raw()
	{
		expect(token_type::keyword_raw);
		expect(token_type::parenthesis_open);

		auto token = expect(token_type::numerical, token_type::identifier);

		expect(token_type::parenthesis_close);

		return std::make_unique<ast::raw_statement>(std::move(token));
	}

	ast::statement parser::parse_define()
	{
		expect(token_type::keyword_define);

		auto identifier = expect(token_type::identifier);
		auto value = expect(token_type::numerical);

		return std::make_unique<ast::define_statement>(
						std::move(identifier),
						std::move(value)
					);
	}

	std::vector<ast::instruction_operand> parser::parse_operands(std::string_view mnemonic)
	{
		const auto count = operands_count.at(mnemonic);

		std::vector<ast::instruction_operand> operands;
		operands.reserve(count);

		for (auto i = 0; i < count; ++i)
		{
			operands.push_back(parse_operand());

			if (i != count - 1)
				expect(token_type::comma);
		}

		return operands;
	}

	ast::statement parser::parse_instruction()
	{
		auto mnemonic = expect(token_type::instruction);

		return std::make_unique<ast::instruction_statement>(
					std::move(mnemonic),
					parse_operands(ccomp::to_string(mnemonic))
				);
	}

	ast::statement parser::parse_procedure()
	{
		expect(token_type::keyword_proc_start);

		auto parse_inner_statement = [&]() -> ast::statement
		{
			if (reached_eof())
				throw std::runtime_error("Unexpected EOF while parsing !");

			switch (token_it->type)
			{
				case token_type::keyword_proc_end: return {};
				case token_type::keyword_define:   return parse_define();
				case token_type::keyword_raw:      return parse_raw();
				case token_type::instruction:      return parse_instruction();
				case token_type::dot:              return parse_label();

				case token_type::keyword_proc_start:
					throw std::runtime_error("Cannot define a procedure inside another");

				default:
					throw parser_exception::unexpected_error(*token_it);
			}
		};

		auto proc_name = expect(token_type::identifier);

		std::vector<ast::statement> inner_statements;

		while (auto block = parse_inner_statement())
			inner_statements.push_back(std::move(block));

		expect(token_type::keyword_proc_end);
		const auto proc_name_end = expect(token_type::identifier);

		if (proc_name_end.data != proc_name.data)
			throw parser_exception::parser_error(
					R"(Different procedure names at lines {} and {} ("{}" != "{}").)",
					proc_name.source_location.line,
					proc_name_end.source_location.line,
					ccomp::to_string(proc_name),
					ccomp::to_string(proc_name_end)
				);

		return std::make_unique<ast::procedure_statement>(
					ccomp::to_string(proc_name),
					std::move(inner_statements)
				);
	}

	ast::statement parser::parse_label()
	{
		expect(token_type::dot);
		auto identifier = expect(token_type::identifier);
		expect(token_type::colon);

		return std::make_unique<ast::label_statement>(std::move(identifier));
	}

	ast::instruction_operand parser::parse_operand()
	{
		auto token = expect(token_type::register_name,
							token_type::identifier,
							token_type::numerical,
							token_type::dot,
							token_type::bracket_open);

		// Is the operand a jump label ?
		if (token.type == token_type::dot)
		{
			auto label = expect(token_type::identifier);
			return ast::instruction_operand(std::move(label));
		}

		// Is there an indirection ?
		if (token.type == token_type::bracket_open)
		{
			auto inner_token = expect(token_type::identifier,
									  token_type::numerical,
									  token_type::register_name);

			expect(token_type::bracket_close);

			return ast::instruction_operand(std::move(inner_token), true);
		}

		return ast::instruction_operand(std::move(token));
	}
}
