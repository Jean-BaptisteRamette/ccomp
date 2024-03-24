#ifndef CCOMP_SANITIZE_VISITOR_HPP
#define CCOMP_SANITIZE_VISITOR_HPP


#include <unordered_set>
#include <stdexcept>
#include <format>
#include <string>
#include <array>

#include <ccomp/source_location.hpp>
#include <ccomp/ast_visitor.hpp>


namespace ccomp::ast
{
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

		void visit(const procedure_statement&) override;
		void visit(const instruction_statement&) override;
		void visit(const define_statement &) override;
		void visit(const raw_statement&) override;
		void visit(const label_statement&) override;

	private:
		void push_scope();
		void pop_scope();
		void register_symbol(const std::string& symbol, const source_location& sym_loc);
		bool symbol_defined(const std::string& symbol);
		bool scope_has_symbol(scope_id scope, const std::string& symbol);

	private:
		static constexpr unsigned char SCOPES_LEVEL = 3u;
		std::array<std::unordered_set<std::string>, SCOPES_LEVEL> scopes;
		scope_id curr_scope_level = 0;
	};


	namespace sanitize_exception
	{
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

		struct undefined_symbol : sanitize_error
		{
			undefined_symbol(const std::string& symbol, const source_location& where)
				: sanitize_error(
					"Sanitizer found undefined symbol \"{}\" at {}.",
					symbol,
					ccomp::to_string(where))
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
