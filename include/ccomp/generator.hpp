#ifndef CCOMP_GENERATOR_HPP
#define CCOMP_GENERATOR_HPP

#include <unordered_map>

#include <ccomp/assembler_error.hpp>
#include <ccomp/ast_visitor.hpp>
#include <ccomp/arch.hpp>
#include <ccomp/ast.hpp>


namespace ccomp
{
	class generator final : public ast::base_visitor
	{
	public:
		generator() = default;
		generator(const generator&) = delete;
		generator(generator&&) = delete;
		generator& operator=(const generator&) = delete;
		generator& operator=(generator&&) = delete;
		~generator() = default;

		[[nodiscard]] std::vector<arch::opcode> generate(const ast::abstract_tree&);

		void visit(const ast::procedure_statement&) override;
		void visit(const ast::instruction_statement&) override;
		void visit(const ast::define_statement&) override;
		void visit(const ast::sprite_statement&) override;
		void visit(const ast::raw_statement&) override;
		void visit(const ast::label_statement&) override;

	private:
		[[nodiscard]] arch::opcode encode_add(const std::vector<ast::instruction_operand>& operands) const;
		[[nodiscard]] arch::opcode encode_sub(const std::vector<ast::instruction_operand>& operands) const;

		void post_visit();
		[[nodiscard]] uint16_t to_integer(const token&) const;

	private:
		std::vector<arch::opcode> binary;
		std::unordered_map<std::string, uint16_t> constants;

		typedef arch::opcode(generator::*encoder)(const std::vector<ast::instruction_operand>&) const;

		const std::unordered_map<std::string_view, encoder> mnemonic_encoders = {
				{ "add", &generator::encode_add },
				{ "sub", &generator::encode_sub }
		};
	};

	namespace generator_exception
	{
		struct generator_invalid_operand_type : assembler_error
		{
			generator_invalid_operand_type()
				: assembler_error("Invalid operands types for instruction")
			{}
		};
	}
}


#endif //CCOMP_GENERATOR_HPP
