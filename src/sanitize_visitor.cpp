#include <ccomp/sanitize_visitor.hpp>
#include <ccomp/statements.hpp>
#include <ccomp/ast.hpp>
#include <format>


namespace ccomp::ast
{
	void sanitize_visitor::traverse(const abstract_tree& ast)
	{
		for (const auto& branch : ast.branches)
			branch->accept(*this);

		post_visit();
	}

	void sanitize_visitor::push_scope()
	{
		++curr_scope_level;
	}

	void sanitize_visitor::pop_scope()
	{
		scopes[curr_scope_level].clear();
		--curr_scope_level;
	}

	void sanitize_visitor::visit(const procedure_statement& statement)
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

	void sanitize_visitor::visit(const instruction_statement& statement)
	{
		for (const auto& [op, _] : statement.operands)
		{
			auto sym = ccomp::to_string(op);

			if (op.type == token_type::identifier && !symbol_defined(sym))
				undefined_labels.insert(std::make_pair(std::move(sym), op.source_location));
		}
	}

	void sanitize_visitor::visit(const label_statement& statement)
	{
		const auto sym = ccomp::to_string(statement.identifier);

		if (undefined_labels.contains(sym))
			undefined_labels.erase(sym);

		if (curr_scope_level > 1)
			pop_scope();

		register_symbol(
				ccomp::to_string(statement.identifier),
				statement.identifier.source_location
			);

		push_scope();
	}

	void sanitize_visitor::visit(const define_statement& statement)
	{
		register_symbol(
				ccomp::to_string(statement.identifier),
				statement.identifier.source_location
			);
	}

	void sanitize_visitor::visit(const raw_statement& statement)
	{
		const auto& sym = statement.opcode;

		if (sym.type == token_type::identifier)
			register_symbol(ccomp::to_string(sym), sym.source_location);
	}

	void sanitize_visitor::register_symbol(const std::string& symbol, const source_location& sym_loc)
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

	bool sanitize_visitor::symbol_defined(const std::string &symbol)
	{
		for (scope_id scp = 0; scp <= curr_scope_level; ++scp)
			if (scope_has_symbol(scp, symbol))
				return true;

		return false;
	}

	bool sanitize_visitor::scope_has_symbol(scope_id scope, const std::string &symbol)
	{
		return scopes[scope].contains(symbol);
	}

	void sanitize_visitor::post_visit()
	{
		if (!undefined_labels.empty())
			throw sanitize_exception::undefined_symbols(undefined_labels);
	}
}
