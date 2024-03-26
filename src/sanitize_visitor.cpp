#include <ccomp/symbol_sanitizer.hpp>
#include <ccomp/statements.hpp>
#include <ccomp/ast.hpp>
#include <format>


namespace ccomp::ast
{
	void symbol_sanitizer::traverse(const abstract_tree& ast)
	{
		for (const auto& branch : ast.branches)
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

	void symbol_sanitizer::visit(const procedure_statement& statement)
	{
		register_symbol(
				ccomp::to_string(statement.name_beg),
				statement.name_beg.source_location
			);

		push_scope();

		for (const auto& inner : statement.inner_statements)
			inner->accept(*this);

		pop_scope();

		if (!undefined_labels.empty())
			throw sanitize_exception::undefined_symbols(undefined_labels);
	}

	void symbol_sanitizer::visit(const instruction_statement& statement)
	{
		for (const auto& [op, _] : statement.operands)
		{
			auto sym = ccomp::to_string(op);

			if (op.type == token_type::identifier && !symbol_defined(sym))
				undefined_labels.insert(std::make_pair(std::move(sym), op.source_location));
		}
	}

	void symbol_sanitizer::visit(const label_statement& statement)
	{
		auto sym = ccomp::to_string(statement.identifier);

		if (undefined_labels.contains(sym))
			undefined_labels.erase(sym);

		register_symbol(
				sym,
				statement.identifier.source_location
			);

		push_scope();

		for (const auto& inner : statement.inner_statements)
			inner->accept(*this);

		pop_scope();
	}

	void symbol_sanitizer::visit(const define_statement& statement)
	{
		register_symbol(
				ccomp::to_string(statement.identifier),
				statement.identifier.source_location
			);
	}

	void symbol_sanitizer::visit(const raw_statement& statement)
	{
		const auto& token = statement.opcode;

		if (token.type == token_type::identifier && !symbol_defined(ccomp::to_string(token)))
			throw sanitize_exception::undefined_symbols(ccomp::to_string(token), token.source_location);
	}

	void symbol_sanitizer::register_symbol(const std::string& symbol, const source_location& sym_loc)
	{
		if (curr_scope_level >= scopes.size())
			throw std::runtime_error(std::format(
						"Could not register symbol \"{}\" because we exceeded the maximum amount of scopes.",
						symbol)
					);

		if (symbol_defined(symbol))
			throw sanitize_exception::already_defined_symbol(symbol, sym_loc);

		scopes[curr_scope_level].insert({symbol, sym_loc});
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

		if (!scope_has_symbol(0, "main"))
			throw sanitize_exception::sanitize_error("Entry-point label \".main\" was not defined.");
	}
}
