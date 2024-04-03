#include <ccomp/arch.hpp>
#include <ccomp/parser.hpp>
#include <ccomp/log.hpp>


namespace ccomp
{

    parser::parser(std::vector<token> &&tokens_list)
        : tokens(std::move(tokens_list)),
		  token_it(std::begin(tokens))
    {}

	token parser::advance()
	{
		if (no_more_tokens())
			throw assembler_error("Expected more tokens before end of file.");

		const token t = *token_it;
		++token_it;

		return t;
	}

    bool parser::no_more_tokens() const
    {
        return token_it == std::end(tokens);
    }

    ast::abstract_tree parser::make_tree()
    {
		std::vector<ast::statement> branches;

		while (auto branch = parse_primary_statement())
			branches.push_back(std::move(branch));

        return ast::abstract_tree(std::move(branches));
    }

	ast::statement parser::parse_primary_statement()
	{
		if (no_more_tokens())
			return {};

		switch (token_it->type)
		{
			case token_type::keyword_define:     return parse_define();
			case token_type::keyword_sprite:     return parse_sprite();
			case token_type::keyword_raw:        return parse_raw();
			case token_type::dot_label:          return parse_label();
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

	ast::statement parser::parse_sprite()
	{
		expect(token_type::keyword_sprite);

		auto identifier = expect(token_type::identifier);
		expect(token_type::bracket_open);

		std::vector<uint8_t> digits;

		do
		{
			auto token = expect(token_type::numerical);
			digits.push_back(token.to_integer());
		}
		while (advance_if(token_type::comma));

		expect(token_type::bracket_close);

		return std::make_unique<ast::sprite_statement>(
						std::move(identifier),
						std::move(digits)
					);
	}

	std::vector<ast::instruction_operand> parser::parse_operands()
	{
		std::vector<ast::instruction_operand> operands;

		while (next_any_of(token_type::identifier,
						   token_type::register_name,
						   token_type::at_label,
						   token_type::numerical,
						   token_type::bracket_open))
		{
			operands.push_back(parse_operand());

			if (!advance_if(token_type::comma))
				break;
		}

		return operands;
	}

	ast::statement parser::parse_instruction()
	{
		auto mnemonic = expect(token_type::instruction);

		return std::make_unique<ast::instruction_statement>(
					std::move(mnemonic),
					parse_operands()
				);
	}

	ast::statement parser::parse_procedure()
	{
		auto parse_inner_statement = [&]() -> ast::statement
		{
			if (no_more_tokens())
				throw assembler_error("Found unexpected EOF before function end.");

			switch (token_it->type)
			{
				case token_type::keyword_proc_end: return {};
				case token_type::keyword_define:   return parse_define();
				case token_type::keyword_raw:      return parse_raw();
				case token_type::instruction:      return parse_instruction();
				case token_type::dot_label:        return parse_label();

				case token_type::keyword_proc_start:
					throw assembler_error("Cannot define a procedure inside another.");

				default:
					throw parser_exception::unexpected_error(*token_it);
			}
		};

		expect(token_type::keyword_proc_start);
		auto proc_name_beg = expect(token_type::identifier);

		std::vector<ast::statement> inner_statements;

		while (auto block = parse_inner_statement())
			inner_statements.push_back(std::move(block));

		expect(token_type::keyword_proc_end);
		auto proc_name_end = expect(token_type::identifier);

		if (proc_name_end.data != proc_name_beg.data)
			throw parser_exception::unmatching_procedure_names(proc_name_beg, proc_name_end);

		return std::make_unique<ast::procedure_statement>(
					std::move(proc_name_beg),
					std::move(proc_name_end),
					std::move(inner_statements)
				);
	}

	ast::statement parser::parse_label()
	{
		auto parse_inner_statement = [&]() -> ast::statement
		{
			if (no_more_tokens())
				return {};

			switch (token_it->type)
			{
				case token_type::keyword_proc_end:
				case token_type::dot_label:
					return {};

				case token_type::keyword_define: return parse_define();
				case token_type::keyword_raw:    return parse_raw();
				case token_type::instruction:    return parse_instruction();

				default:
					throw parser_exception::unexpected_error(*token_it);
			}
		};

		expect(token_type::dot_label);
		auto identifier = expect(token_type::identifier);
		expect(token_type::colon);

		std::vector<ast::statement> inner_statements;

		while (auto block = parse_inner_statement())
			inner_statements.push_back(std::move(block));

		return std::make_unique<ast::label_statement>(
					std::move(identifier),
					std::move(inner_statements)
				);
	}

	ast::instruction_operand parser::parse_operand()
	{
		auto token = expect(token_type::register_name,
							token_type::identifier,
							token_type::numerical,
							token_type::at_label,
							token_type::bracket_open);

		// Is the operand a jump label ?
		if (token.type == token_type::at_label)
		{
			auto label = expect(token_type::identifier);
			return ast::instruction_operand(std::move(label), false, true);
		}

		// Is there an indirection ?
		if (token.type == token_type::bracket_open)
		{
			auto inner_token = expect(token_type::identifier,
									  token_type::numerical);

			expect(token_type::bracket_close);

			return ast::instruction_operand(std::move(inner_token), true);
		}

		return ast::instruction_operand(std::move(token));
	}
}
