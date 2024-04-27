#ifndef CHASM_GENERATOR_HPP
#define CHASM_GENERATOR_HPP

#include <unordered_map>

#include <chasm/chasm_exception.hpp>
#include <chasm/ast_visitor.hpp>
#include <chasm/config.hpp>
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
		void visit(const ast::config_statement&) override;
		void visit(const ast::sprite_statement&) override;
		void visit(const ast::raw_statement&) override;
		void visit(const ast::label_statement&) override;

	private:
		void emit_byte(uint8_t b);
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
		[[nodiscard]] arch::opcode encode_saverpl(const ast::instruction_statement&);
		[[nodiscard]] arch::opcode encode_loadrpl(const ast::instruction_statement&);

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
		config cfg;

		std::string current_proc_name;

		typedef arch::opcode(generator::*encoder)(const ast::instruction_statement&);
		typedef std::vector<arch::opcode>(generator::*pseudo_encoder)(const ast::instruction_statement&);

		const std::unordered_map<arch::instruction_id, encoder> mnemonic_encoders = {
				{ arch::instruction_id::ADD,   &generator::encode_add },
				{ arch::instruction_id::SUB,   &generator::encode_sub },
				{ arch::instruction_id::SUBA,  &generator::encode_suba },
				{ arch::instruction_id::OR,    &generator::encode_or },
				{ arch::instruction_id::AND,   &generator::encode_and },
				{ arch::instruction_id::XOR,   &generator::encode_xor },
				{ arch::instruction_id::SHR,   &generator::encode_shr },
				{ arch::instruction_id::SHL,   &generator::encode_shl },
				{ arch::instruction_id::RDUMP, &generator::encode_rdump },
				{ arch::instruction_id::RLOAD, &generator::encode_rload },
				{ arch::instruction_id::MOV,   &generator::encode_mov },
				{ arch::instruction_id::DRAW,  &generator::encode_draw },
				{ arch::instruction_id::CLS,   &generator::encode_cls },
				{ arch::instruction_id::RAND,  &generator::encode_rand },
				{ arch::instruction_id::BCD,   &generator::encode_bcd },
				{ arch::instruction_id::WKEY,  &generator::encode_wkey },
				{ arch::instruction_id::SKE,   &generator::encode_ske },
				{ arch::instruction_id::SKNE,  &generator::encode_skne },
				{ arch::instruction_id::RET,   &generator::encode_ret },
				{ arch::instruction_id::JMP,   &generator::encode_jmp },
				{ arch::instruction_id::CALL,  &generator::encode_call },
				{ arch::instruction_id::SE,    &generator::encode_se },
				{ arch::instruction_id::SNE,   &generator::encode_sne },
				{ arch::instruction_id::INC,   &generator::encode_inc },
				{ arch::instruction_id::LDF,   &generator::encode_ldf },
		};

		const std::unordered_map<arch::instruction_id, encoder> super_mnemonic_encoders = {
				{ arch::instruction_id::EXIT,    &generator::encode_exit },
				{ arch::instruction_id::SCRD,    &generator::encode_scrd },
				{ arch::instruction_id::SCRL,    &generator::encode_scrl },
				{ arch::instruction_id::SCRR,    &generator::encode_scrr },
				{ arch::instruction_id::HIGH,    &generator::encode_high },
				{ arch::instruction_id::LOW,     &generator::encode_low },
				{ arch::instruction_id::LDFS,    &generator::encode_ldfs },
				{ arch::instruction_id::SAVERPL, &generator::encode_saverpl },
				{ arch::instruction_id::LOADRPL, &generator::encode_loadrpl },
		};


		const std::unordered_map<arch::instruction_id, pseudo_encoder> pseudo_mnemonic_encoders = {
				{arch::instruction_id::SWP, &generator::encode_swp }
		};
	};

	namespace generator_exception
	{
		struct invalid_operand_type : chasm_exception
		{
			explicit invalid_operand_type(const ast::instruction_statement& inst)
				: chasm_exception("Invalid operand type for instruction \"{}\" at {}.",
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

		struct invalid_operands_count : chasm_exception
		{
			explicit invalid_operands_count(const ast::instruction_statement& inst,
											std::initializer_list<int> expected_counts)
				: chasm_exception("Invalid operands count for instruction \"{}\" at {}.\n"
								  "Expected operands count to be among {} but {} operands were provided",
								  inst.mnemonic.to_string(),
								  to_string(inst.mnemonic.source_location),
								  to_string(expected_counts),
								  inst.operands.size())
			{}
		};

		struct invalid_immediate_format : chasm_exception
		{
			invalid_immediate_format(arch::imm imm, arch::imm_format bit_format)
				: chasm_exception("Immediate value {} is too big for expected operand format of {} bits.",
								  imm,
								  static_cast<int>(bit_format))
			{}
		};
	}
}


#endif //CHASM_GENERATOR_HPP
