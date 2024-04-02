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
		[[nodiscard]] arch::opcode encode_suba(const std::vector<ast::instruction_operand>& operands) const;
		[[nodiscard]] arch::opcode encode_or(const std::vector<ast::instruction_operand>& operands) const;
		[[nodiscard]] arch::opcode encode_and(const std::vector<ast::instruction_operand>& operands) const;
		[[nodiscard]] arch::opcode encode_xor(const std::vector<ast::instruction_operand>& operands) const;
		[[nodiscard]] arch::opcode encode_shr(const std::vector<ast::instruction_operand>& operands) const;
		[[nodiscard]] arch::opcode encode_shl(const std::vector<ast::instruction_operand>& operands) const;
		[[nodiscard]] arch::opcode encode_rdump(const std::vector<ast::instruction_operand>& operands) const;
		[[nodiscard]] arch::opcode encode_rload(const std::vector<ast::instruction_operand>& operands) const;
		[[nodiscard]] arch::opcode encode_mov(const std::vector<ast::instruction_operand>& operands) const;
		[[nodiscard]] arch::opcode encode_swp(const std::vector<ast::instruction_operand>& operands) const;
		[[nodiscard]] arch::opcode encode_draw(const std::vector<ast::instruction_operand>& operands) const;
		[[nodiscard]] arch::opcode encode_cls(const std::vector<ast::instruction_operand>& operands) const;
		[[nodiscard]] arch::opcode encode_rand(const std::vector<ast::instruction_operand>& operands) const;
		[[nodiscard]] arch::opcode encode_bcd(const std::vector<ast::instruction_operand>& operands) const;
		[[nodiscard]] arch::opcode encode_wkey(const std::vector<ast::instruction_operand>& operands) const;
		[[nodiscard]] arch::opcode encode_ske(const std::vector<ast::instruction_operand>& operands) const;
		[[nodiscard]] arch::opcode encode_skne(const std::vector<ast::instruction_operand>& operands) const;
		[[nodiscard]] arch::opcode encode_ret(const std::vector<ast::instruction_operand>& operands) const;
		[[nodiscard]] arch::opcode encode_jmp(const std::vector<ast::instruction_operand>& operands) const;
		[[nodiscard]] arch::opcode encode_call(const std::vector<ast::instruction_operand>& operands) const;
		[[nodiscard]] arch::opcode encode_se(const std::vector<ast::instruction_operand>& operands) const;
		[[nodiscard]] arch::opcode encode_sne(const std::vector<ast::instruction_operand>& operands) const;
		[[nodiscard]] arch::opcode encode_inc(const std::vector<ast::instruction_operand>& operands) const;

		void post_visit();

		[[nodiscard]] arch::imm operand2imm(const token& token,
											arch::imm_format imm_width = arch::imm_format::imm8) const;

		[[nodiscard]] arch::imm operand2imm(const ast::instruction_operand& operand,
											arch::imm_format type = arch::imm_format::imm8) const;

	private:
		std::vector<arch::opcode> binary;
		std::unordered_map<std::string, arch::imm> constants;

		typedef arch::opcode(generator::*encoder)(const std::vector<ast::instruction_operand>&) const;

		const std::unordered_map<std::string_view, encoder> mnemonic_encoders = {
				{ "add", &generator::encode_add },
				{ "sub", &generator::encode_sub },
				{ "suba", &generator::encode_suba },
				{ "or", &generator::encode_or },
				{ "and", &generator::encode_and },
				{ "xor", &generator::encode_xor },
				{ "shr", &generator::encode_shr },
				{ "shl", &generator::encode_shl },
				{ "rdump", &generator::encode_rdump },
				{ "rload", &generator::encode_rload },
				{ "mov", &generator::encode_mov },
				{ "swp", &generator::encode_swp },
				{ "draw", &generator::encode_draw },
				{ "cls", &generator::encode_cls },
				{ "rand", &generator::encode_rand },
				{ "bcd", &generator::encode_bcd },
				{ "wkey", &generator::encode_wkey },
				{ "ske", &generator::encode_ske },
				{ "skne", &generator::encode_skne },
				{ "ret", &generator::encode_ret },
				{ "jmp", &generator::encode_jmp },
				{ "call", &generator::encode_call },
				{ "se", &generator::encode_se },
				{ "sne", &generator::encode_sne },
				{ "inc", &generator::encode_inc },
		};
	};

	namespace generator_exception
	{
		struct invalid_operand_type : assembler_error
		{
			invalid_operand_type()
				: assembler_error("Invalid operands types for instruction.")
			{}
		};

		struct too_many_operands : assembler_error
		{
			explicit too_many_operands(const ast::instruction_statement& statement)
				: assembler_error("CHIP-8 instructions can have a maximum of {} operands per instruction.\n"
								  "Constraint violated for instruction \"{}\" at {}.",
								  arch::MAX_OPERANDS,
								  statement.mnemonic.to_string(),
								  to_string(statement.mnemonic.source_location))
			{}
		};

		struct invalid_immediate_format : assembler_error
		{
			invalid_immediate_format(arch::imm imm, arch::imm_format bit_format)
				: assembler_error("Immediate value {} is too big for expected operand format of {} bits.",
								  imm,
								  static_cast<int>(bit_format))
			{}
		};
	}
}


#endif //CCOMP_GENERATOR_HPP
