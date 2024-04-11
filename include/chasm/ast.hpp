#ifndef CHASM_AST_HPP
#define CHASM_AST_HPP


#include <vector>

#include <chasm/ast_visitor.hpp>
#include <chasm/statements.hpp>


namespace chasm::ast
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


#endif //CHASM_AST_HPP
