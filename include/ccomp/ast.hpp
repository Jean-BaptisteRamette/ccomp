#ifndef CCOMP_AST_HPP
#define CCOMP_AST_HPP


#include <vector>

#include <ccomp/ast_visitor.hpp>
#include <ccomp/statements.hpp>


namespace ccomp::ast
{
    class abstract_tree
    {
	public:
        explicit abstract_tree(std::vector<ast::statement>&& branches);

		[[nodiscard]] std::vector<uint16_t> generate();
		[[nodiscard]] const std::vector<ast::statement>& branches() const;

	private:
		void sanitize() const;

	private:
		std::vector<ast::statement> statements {};
    };
}


#endif //CCOMP_AST_HPP
