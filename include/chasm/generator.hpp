#ifndef CHASM_GENERATOR_HPP
#define CHASM_GENERATOR_HPP

#include <unordered_map>

#include <chasm/assembler_error.hpp>
#include <chasm/ast_visitor.hpp>
#include <chasm/arch.hpp>
#include <chasm/ast.hpp>


namespace chasm
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

		[[nodiscard]] std::vector<uint8_t> generate(const ast::abstract_tree&);

		void visit(const ast::procedure_statement&) override;
		void visit(const ast::instruction_statement&) override;
		void visit(const ast::define_statement&) override;
		void visit(const ast::sprite_statement&) override;
		void visit(const ast::raw_statement&) override;
		void visit(const ast::label_statement&) override;

	private:
		void emit_opcode(arch::opcode opcode);
		void emit_opcodes(const std::vector<arch::opcode>& opcodes);

		void register_constant(std::string&& symbol, arch::imm value);
		void register_sprite(std::string&& symbol, const arch::sprite& sprite);
		void register_symbol_addr(std::string symbol);
		void register_patch_location(std::string&& symbol);

		[[nodiscard]] arch::opcode encode_add(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_sub(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_suba(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_or(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_and(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_xor(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_shr(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_shl(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_rdump(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_rload(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_mov(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_draw(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_cls(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_rand(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_bcd(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_wkey(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_ske(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_skne(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_ret(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_jmp(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_call(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_se(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_sne(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_inc(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_ldf(const ast::instruction_statement&);

		[[nodiscard]] arch::opcode encode_exit(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_scrd(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_scrl(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_scrr(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_high(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_low(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_ldfs(const ast::instruction_statement&);

		[[nodiscard]] std::vector<arch::opcode> encode_swp(const ast::instruction_statement&);

		void post_visit();

		[[nodiscard]] arch::imm operand2imm(const token& token,
											arch::imm_format imm_width = arch::imm_format::fmt_imm8) const;

		[[nodiscard]] arch::imm operand2imm(const ast::instruction_operand& operand,
											arch::imm_format type = arch::imm_format::fmt_imm8) const;

	private:

		struct address_patch
		{
			size_t location;
			std::string sym;
		};

		std::vector<uint8_t> binary;
		std::vector<address_patch> patches;
		std::unordered_map<std::string, arch::addr> sym_addresses;
		std::unordered_map<std::string, arch::imm> constants;
		std::unordered_map<std::string, arch::sprite> sprites;

		std::string current_proc_name;

		typedef arch::opcode(generator::*encoder)(const ast::instruction_statement&);
		typedef std::vector<arch::opcode>(generator::*pseudo_encoder)(const ast::instruction_statement&);

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
				{ "ldf", &generator::encode_ldf },
		};

		const std::unordered_map<std::string_view, encoder> super_mnemonic_encoders = {
				{ "exit", &generator::encode_exit },
				{ "scrd", &generator::encode_scrd },
				{ "scrl", &generator::encode_scrl },
				{ "scrr", &generator::encode_scrr },
				{ "high", &generator::encode_high },
				{ "low", &generator::encode_low },
				{ "ldfs", &generator::encode_ldfs },
		};


		const std::unordered_map<std::string_view, pseudo_encoder> pseudo_mnemonic_encoders = {
				{ "swp", &generator::encode_swp }
		};
	};

	namespace generator_exception
	{
		struct invalid_operand_type : assembler_error
		{
			explicit invalid_operand_type(const ast::instruction_statement& inst)
				: assembler_error("Invalid operand type for instruction \"{}\" at {}.",
								  inst.mnemonic.to_string(),
								  to_string(inst.mnemonic.source_location))
			{}
		};

		[[nodiscard]]
		inline std::string to_string(std::initializer_list<int> list)
		{
			std::string joined;

			for (const auto val : list)
			{
				if (!joined.empty())
					joined += ", ";

				joined += std::to_string(val);
			}

			return '(' + joined + ')';
		}

		struct invalid_operands_count : assembler_error
		{
			explicit invalid_operands_count(const ast::instruction_statement& inst,
											std::initializer_list<int> expected_counts)
				: assembler_error("Invalid operands count for instruction \"{}\" at {}.\n"
								  "Expected operands count to be among {} but {} operands were provided",
								  inst.mnemonic.to_string(),
								  to_string(inst.mnemonic.source_location),
								  to_string(expected_counts),
								  inst.operands.size())
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


#endif //CHASM_GENERATOR_HPP
