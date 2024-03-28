#ifndef CCOMP_AST_HPP
#define CCOMP_AST_HPP


#include <string_view>
#include <concepts>
#include <memory>
#include <vector>

#include <ccomp/ast_visitor.hpp>
#include <ccomp/statements.hpp>


namespace ccomp::ast
{
    class abstract_tree
    {
	public:
        explicit abstract_tree(std::vector<ast::statement>&& statements_list);

		[[nodiscard]] std::vector<uint16_t> generate() const;

	public:
		const std::vector<ast::statement> branches {};

	private:
		void sanitize() const;
    };
}


#endif //CCOMP_AST_HPP
