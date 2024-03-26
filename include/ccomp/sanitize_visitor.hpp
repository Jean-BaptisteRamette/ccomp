#ifndef CCOMP_SANITIZE_VISITOR_HPP
#define CCOMP_SANITIZE_VISITOR_HPP


#include <unordered_map>
#include <stdexcept>
#include <format>
#include <string>
#include <array>

#include <ccomp/source_location.hpp>
#include <ccomp/ast_visitor.hpp>


namespace ccomp::ast
{
	using symbol_set = std::unordered_map<std::string, source_location>;

	struct abstract_tree;

	class sanitize_visitor : public base_visitor
	{
		using scope_id = unsigned char;

	public:
		sanitize_visitor() = default;
		sanitize_visitor(const sanitize_visitor&) = delete;
		sanitize_visitor(sanitize_visitor&&) = delete;
		sanitize_visitor& operator=(const sanitize_visitor&) = delete;
		sanitize_visitor& operator=(sanitize_visitor&&) = delete;
		~sanitize_visitor() = default;

		void traverse(const abstract_tree&);

		void visit(const procedure_statement&) override;
		void visit(const instruction_statement&) override;
		void visit(const define_statement&) override;
		void visit(const raw_statement&) override;
		void visit(const label_statement&) override;


	private:
		void post_visit();
		void push_scope();
		void pop_scope();
		void register_symbol(const std::string& symbol, const source_location& sym_loc);
		bool symbol_defined(const std::string& symbol);
		bool scope_has_symbol(scope_id scope, const std::string& symbol);

	private:

		static constexpr unsigned char SCOPES_LEVEL = 3u;
		std::array<symbol_set, SCOPES_LEVEL> scopes;
		scope_id curr_scope_level = 0;
		symbol_set undefined_labels;
	};


	namespace sanitize_exception
	{
		inline std::string symbols_to_string(const symbol_set& symbols)
		{
			std::string joined;

			for (const auto& [sym, loc] : symbols)
				joined += std::format("\t --- \"{}\" at {}\n", sym, ccomp::to_string(loc));

			return joined;
		}

		struct sanitize_error : std::runtime_error
		{
			explicit sanitize_error(std::string_view message)
				: std::runtime_error(message.data())
			{}

			template<typename ...Args>
			explicit sanitize_error(std::string_view fmt_message, Args&&... args)
				: std::runtime_error(std::vformat(fmt_message, std::make_format_args(args...)))
			{}
		};

		struct undefined_symbols : sanitize_error
		{
			explicit undefined_symbols(const std::string& symbol, const source_location& where)
				: sanitize_error(
						"Sanitizer found undefined symbol: \"{}\" at {}.",
						symbol,
						ccomp::to_string(where))
			{}

			explicit undefined_symbols(const symbol_set& symbols)
				: sanitize_error(
					"Sanitizer found undefined symbols:\n{}",
					symbols_to_string(symbols))
			{}
		};

		struct already_defined_symbol : sanitize_error
		{
			already_defined_symbol(const std::string& symbol, const source_location& where)
				: sanitize_error(
					"Sanitizer found an already defined symbol \"{}\" at {}.",
					symbol,
					ccomp::to_string(where))
			{}
		};
	}
}


#endif //CCOMP_SANITIZE_VISITOR_HPP
