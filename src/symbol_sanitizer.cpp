#include <chasm/symbol_sanitizer.hpp>
#include <chasm/statements.hpp>
#include <format>


namespace chasm
{
	void symbol_sanitizer::traverse(const ast::abstract_tree& ast)
	{
		for (const auto& branch : ast.branches())
			branch->accept(*this);

		post_visit();
	}

	void symbol_sanitizer::push_scope()
	{
		++curr_scope_level;
	}

	void symbol_sanitizer::pop_scope()
	{
		scopes[curr_scope_level].clear();
		--curr_scope_level;
	}

	void symbol_sanitizer::visit(const ast::procedure_statement& statement)
	{
		auto sym = statement.name_beg.to_string();

		if (undefined_procs.contains(sym))
			undefined_procs.erase(sym);

		register_symbol(
				std::move(sym),
				statement.name_beg.source_location
			);

		push_scope();

		for (const auto& inner : statement.inner_statements)
			inner->accept(*this);

		pop_scope();

		if (!undefined_labels.empty())
			throw sanitize_exception::undefined_symbols(undefined_labels);
	}

	void symbol_sanitizer::visit(const ast::instruction_statement& statement)
	{
		for (const auto& operand : statement.operands)
		{
			const auto& operand_token = operand.operand;

			auto sym = operand_token.to_string();

			if (operand_token.type == token_type::identifier && !symbol_defined(sym))
			{
				if (statement.mnemonic.to_string() == "jmp")
					undefined_labels.insert(std::make_pair(std::move(sym), operand_token.source_location));
				else if (statement.mnemonic.to_string() == "call")
					undefined_procs.insert(std::make_pair(std::move(sym), operand_token.source_location));
				else
					throw sanitize_exception::undefined_symbols(sym, operand_token.source_location);
			}
		}
	}

	void symbol_sanitizer::visit(const ast::label_statement& statement)
	{
		auto sym = statement.identifier.to_string();

		if (undefined_labels.contains(sym))
			undefined_labels.erase(sym);

		register_symbol(
				std::move(sym),
				statement.identifier.source_location
			);

		push_scope();

		for (const auto& inner : statement.inner_statements)
			inner->accept(*this);

		pop_scope();
	}

	void symbol_sanitizer::visit(const ast::define_statement& statement)
	{
		register_symbol(
				statement.identifier.to_string(),
				statement.identifier.source_location
			);
	}

	void symbol_sanitizer::visit(const ast::sprite_statement& statement)
	{
		if (curr_scope_level != 0)
			throw assembler_error(
					"Sprite \"{}\" at line {} must have a global scope",
					statement.identifier.to_string(),
					statement.source_line_beg());

		register_symbol(
				statement.identifier.to_string(),
				statement.identifier.source_location
			);
	}

	void symbol_sanitizer::visit(const ast::raw_statement& statement)
	{
		if (curr_scope_level == 0)
			throw assembler_error("Invalid scope level for raw statement at line {}", statement.source_line_beg());

		const auto& token = statement.opcode;

		if (token.type == token_type::identifier && !symbol_defined(token.to_string()))
			throw sanitize_exception::undefined_symbols(token.to_string(), token.source_location);
	}

	void symbol_sanitizer::register_symbol(std::string&& symbol, const source_location& sym_loc)
	{
		if (curr_scope_level >= scopes.size())
			throw assembler_error(
						"Invalid scope level for symbol \"{}\".",
						symbol);

		if (symbol_defined(symbol))
			throw sanitize_exception::already_defined_symbol(symbol, sym_loc);

		scopes[curr_scope_level].insert({std::move(symbol), sym_loc});
	}

	bool symbol_sanitizer::symbol_defined(const std::string &symbol)
	{
		for (scope_id scp = 0; scp <= curr_scope_level; ++scp)
			if (scope_has_symbol(scp, symbol))
				return true;

		return false;
	}

	bool symbol_sanitizer::scope_has_symbol(scope_id scope, const std::string &symbol)
	{
		return scopes[scope].contains(symbol);
	}

	void symbol_sanitizer::post_visit()
	{
		if (!undefined_labels.empty())
			throw sanitize_exception::undefined_symbols(undefined_labels);

		if (!undefined_procs.empty())
			throw sanitize_exception::undefined_symbols(undefined_procs);

		if (!scope_has_symbol(0, "main"))
			throw assembler_error("Entry-point label \".main\" was not defined.");
	}
}
