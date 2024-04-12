#ifndef CHASM_STATEMENTS_HPP
#define CHASM_STATEMENTS_HPP

#include <chasm/ast_visitor.hpp>
#include <chasm/lexer.hpp>
#include <vector>


namespace chasm::ast
{
	enum class statement_priority
	{
		procedure = 0,
		first
	};

    struct base_statement
    {
		base_statement() = default;
		base_statement(const base_statement&) = delete;
		base_statement(base_statement&&) = delete;
		base_statement& operator=(const base_statement&) = delete;
		base_statement& operator=(base_statement&&) = delete;

		virtual ~base_statement() = default;

		[[nodiscard]] virtual statement_priority priority() const { return statement_priority::first; }

		virtual void accept(base_visitor&) const = 0;

		[[nodiscard]] virtual size_t source_line_beg() const = 0;
		[[nodiscard]] virtual size_t source_line_end() const = 0;


	};

	using statement = std::unique_ptr<base_statement>;

    struct procedure_statement : base_statement
    {
        procedure_statement(token name_beg_, token name_end_, std::vector<statement> inner_statements_)
            : base_statement(),
			  name_beg(std::move(name_beg_)),
			  name_end(std::move(name_end_)),
              inner_statements(std::move(inner_statements_))
        {}

		[[nodiscard]] statement_priority priority() const override { return statement_priority::procedure; }

		void accept(base_visitor& visitor) const override { return visitor.visit(*this); }

		[[nodiscard]] size_t source_line_beg() const override { return name_beg.source_location.line; }
		[[nodiscard]] size_t source_line_end() const override { return name_end.source_location.line; }

        const token name_beg;
		const token name_end;

		// raw, define, instructions and label statements
        const std::vector<statement> inner_statements;
    };

    class instruction_operand
    {
		enum class type
		{
			immediate,
			reg,
			indirection,
			label,
			procedure,
			sprite
		};

		explicit instruction_operand(token operand_, type t)
				: operand(std::move(operand_)),
				  type_(t)
		{}

	public:
		static instruction_operand make_immediate(token operand_)
		{
			return instruction_operand(std::move(operand_), type::immediate);
		}

		static instruction_operand make_reg(token operand_)
		{
			return instruction_operand(std::move(operand_), type::reg);
		}

		static instruction_operand make_label(token operand_)
		{
			return instruction_operand(std::move(operand_), type::label);
		}

		static instruction_operand make_proc(token operand_)
		{
			return instruction_operand(std::move(operand_), type::procedure);
		}

		static instruction_operand make_sprite(token operand_)
		{
			return instruction_operand(std::move(operand_), type::sprite);
		}

		static instruction_operand make_indirect(token operand_)
		{
			return instruction_operand(std::move(operand_), type::indirection);
		}

		instruction_operand(instruction_operand&& other) noexcept
			: operand(std::move(other.operand)),
			  type_(other.type_)
		{}

		instruction_operand& operator=(instruction_operand&& other) noexcept
		{
			operand = std::move(other.operand);
			type_ = other.type_;

			return *this;
		}

		[[nodiscard]] arch::operand_type arch_type() const
		{
			auto regname2optype = [](const auto& name) -> arch::operand_type
			{
				if (name == "ar") return arch::operand_type::reg_ar;
				if (name == "st") return arch::operand_type::reg_st;
				if (name == "dt") return arch::operand_type::reg_dt;

				return arch::operand_type::reg_rx;
			};

			if (is_reg())
				return regname2optype(operand.to_string());
			else if (has_indirection())
				return arch::operand_type::address_indirect;
			else if (is_label() || is_procedure() || is_sprite())
				return arch::operand_type::address;
			else
				return arch::operand_type::immediate;
		}

		[[nodiscard]] std::string reg_name() const
		{
			return operand.to_string();
		}

		[[nodiscard]] bool is_reg() const
		{
			return type_ == type::reg;
		}

		[[nodiscard]] bool has_indirection() const
		{
			return type_ == type::indirection;
		}

		[[nodiscard]] bool is_label() const
		{
			return type_ == type::label;
		}

		[[nodiscard]] bool is_procedure() const
		{
			return type_ == type::procedure;
		}

		[[nodiscard]] bool is_sprite() const
		{
			return type_ == type::sprite;
		}

		token operand;

	private:
		type type_;
    };

    struct instruction_statement : base_statement
    {
		instruction_statement(token mnemonic_, std::vector<instruction_operand> operands_)
			: base_statement(),
			  mnemonic(std::move(mnemonic_)),
			  operands(std::move(operands_))
		{}

		void accept(base_visitor& visitor) const override { return visitor.visit(*this); }

		[[nodiscard]] size_t source_line_beg() const override {  return mnemonic.source_location.line; }
		[[nodiscard]] size_t source_line_end() const override {  return mnemonic.source_location.line; }

    	const token mnemonic;

		// chip-8 instructions 0 to 3 operands
		const std::vector<instruction_operand> operands;
    };

    struct define_statement : base_statement
    {
        define_statement(token identifier_, token value_)
            : base_statement(),
			  identifier(std::move(identifier_)),
			  value(std::move(value_))
        {}

		void accept(base_visitor& visitor) const override { return visitor.visit(*this); }

		[[nodiscard]] size_t source_line_beg() const override {  return identifier.source_location.line; }
		[[nodiscard]] size_t source_line_end() const override {  return value.source_location.line; }

        const token identifier;
        const token value;
    };

	struct sprite_statement : base_statement
	{
		sprite_statement(token identifier_, const arch::sprite& sprite_)
			: base_statement(),
			  identifier(std::move(identifier_)),
			  sprite(sprite_)
		{}

		void accept(base_visitor& visitor) const override { return visitor.visit(*this); }

		[[nodiscard]] size_t source_line_beg() const override { return identifier.source_location.line; }
		[[nodiscard]] size_t source_line_end() const override { return identifier.source_location.line; }

		const token identifier;
		const arch::sprite sprite;
	};

    struct raw_statement : base_statement
    {
		explicit raw_statement(token opcode_)
			: base_statement(),
			  opcode(std::move(opcode_))
		{}

		void accept(base_visitor& visitor) const override { return visitor.visit(*this); }

		[[nodiscard]] size_t source_line_beg() const override {  return opcode.source_location.line; }
		[[nodiscard]] size_t source_line_end() const override {  return opcode.source_location.line; }

        const token opcode;
    };

	struct label_statement : base_statement
	{
		explicit label_statement(token identifier_, std::vector<statement> inner_statements_)
			: base_statement(),
			  identifier(std::move(identifier_)),
			  inner_statements(std::move(inner_statements_))
		{}

		void accept(base_visitor& visitor) const override { return visitor.visit(*this); }

		[[nodiscard]] size_t source_line_beg() const override {  return identifier.source_location.line; }
		[[nodiscard]] size_t source_line_end() const override {  return identifier.source_location.line; }

		const token identifier;

		// raw, define, and instructions statements
		const std::vector<statement> inner_statements;
	};
}

#endif //CHASM_STATEMENTS_HPP
