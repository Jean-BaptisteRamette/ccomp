#ifndef CCOMP_INSTRUCTION_GENERATOR_HPP
#define CCOMP_INSTRUCTION_GENERATOR_HPP


#include <ccomp/assembler_error.hpp>
#include <ccomp/ast_visitor.hpp>
#include <ccomp/ast.hpp>


namespace ccomp
{
	class instruction_generator final : public ast::base_visitor
	{
	public:
		instruction_generator() = default;
		instruction_generator(const instruction_generator&) = delete;
		instruction_generator(instruction_generator&&) = delete;
		instruction_generator& operator=(const instruction_generator&) = delete;
		instruction_generator& operator=(instruction_generator&&) = delete;
		~instruction_generator() = default;

		[[nodiscard]] std::vector<uint16_t> generate(const ast::abstract_tree&);

		void visit(const ast::procedure_statement&) override;
		void visit(const ast::instruction_statement&) override;
		void visit(const ast::define_statement&) override;
		void visit(const ast::raw_statement&) override;
		void visit(const ast::label_statement&) override;

	private:
		void post_visit();
		[[nodiscard]] bool operands_types_ok(const ast::instruction_statement&) const;

		std::vector<uint16_t> binary;
	};

	namespace generator_exception
	{
		struct generator_invalid_operands_types : assembler_error
		{
			// TODO
			generator_invalid_operands_types()
				: assembler_error("Invalid operands types for instruction")
			{}
		};
	}
}


#endif //CCOMP_INSTRUCTION_GENERATOR_HPP
