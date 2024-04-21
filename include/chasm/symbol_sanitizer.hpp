#ifndef CHASM_SYMBOL_SANITIZER_HPP
#define CHASM_SYMBOL_SANITIZER_HPP


#include <unordered_map>
#include <stdexcept>
#include <format>
#include <string>
#include <array>

#include <chasm/chasm_exception.hpp>
#include <chasm/source_location.hpp>
#include <chasm/ast_visitor.hpp>
#include <chasm/ast.hpp>


namespace chasm
{
	using symbol_set = std::unordered_map<std::string, source_location>;


	class symbol_sanitizer final : public ast::base_visitor
	{
		using scope_id = unsigned char;

	public:
		symbol_sanitizer() = default;
		symbol_sanitizer(const symbol_sanitizer&) = delete;
		symbol_sanitizer(symbol_sanitizer&&) = delete;
		symbol_sanitizer& operator=(const symbol_sanitizer&) = delete;
		symbol_sanitizer& operator=(symbol_sanitizer&&) = delete;
		~symbol_sanitizer() = default;

		void traverse(const ast::abstract_tree&);

		void visit(const ast::procedure_statement&) override;
		void visit(const ast::instruction_statement&) override;
		void visit(const ast::define_statement&) override;
		void visit(const ast::sprite_statement&) override;
		void visit(const ast::raw_statement&) override;
		void visit(const ast::label_statement&) override;


	private:
		void post_visit();
		void push_scope();
		void pop_scope();
		void register_symbol(std::string&& symbol, const source_location& sym_loc);
		bool symbol_defined(const std::string& symbol);
		bool scope_has_symbol(scope_id scope, const std::string& symbol);

	private:

		static constexpr unsigned char SCOPES_LEVEL = 3u;
		std::array<symbol_set, SCOPES_LEVEL> scopes;
		scope_id curr_scope_level = 0;
		symbol_set undefined_labels;
		symbol_set undefined_procs;
	};


	namespace sanitize_exception
	{
		inline std::string symbols_to_string(const symbol_set& symbols)
		{
			std::string joined;

			for (const auto& [sym, loc] : symbols)
				joined += std::format("\t --- \"{}\" at {}\n", sym, chasm::to_string(loc));

			return joined;
		}

		struct undefined_symbols : chasm_exception
		{
			explicit undefined_symbols(const std::string& symbol, const source_location& where)
				: chasm_exception(
						"Sanitizer found an undefined symbol: \"{}\" at {}.",
						symbol,
						chasm::to_string(where))
			{}

			explicit undefined_symbols(const symbol_set& symbols)
				: chasm_exception(
					"Sanitizer found the following undefined symbols:\n{}",
					symbols_to_string(symbols))
			{}
		};

		struct already_defined_symbol : chasm_exception
		{
			already_defined_symbol(const std::string& symbol, const source_location& where)
				: chasm_exception(
					"Sanitizer found an already defined symbol \"{}\" at {}.",
					symbol,
					chasm::to_string(where))
			{}
		};
	}
}


#endif //CHASM_SYMBOL_SANITIZER_HPP
