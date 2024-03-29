#ifndef CCOMP_STATEMENTS_HPP
#define CCOMP_STATEMENTS_HPP

#include <ccomp/ast_visitor.hpp>
#include <ccomp/lexer.hpp>
#include <vector>


namespace ccomp::ast
{
    struct base_statement
    {
		base_statement() = default;
		base_statement(const base_statement&) = delete;
		base_statement(base_statement&&) = delete;
		base_statement& operator=(const base_statement&) = delete;
		base_statement& operator=(base_statement&&) = delete;

		virtual void accept(base_visitor&) const = 0;

		[[nodiscard]] virtual size_t source_line_beg() const = 0;
		[[nodiscard]] virtual size_t source_line_end() const = 0;

        virtual ~base_statement() = default;
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

		void accept(base_visitor& visitor) const override { return visitor.visit(*this); }

		[[nodiscard]] size_t source_line_beg() const override { return name_beg.source_location.line; }
		[[nodiscard]] size_t source_line_end() const override { return name_end.source_location.line; }

        const token name_beg;
		const token name_end;

		// raw, define, instructions and label statements
        const std::vector<statement> inner_statements;
    };

    struct instruction_operand
    {
		explicit instruction_operand(token operand_, bool indirection_ = false)
			: operand(std::move(operand_)),
			  indirection(indirection_)
		{}

		instruction_operand(instruction_operand&& other) noexcept
			: indirection(other.indirection),
			  operand(std::move(other.operand))
		{}

		instruction_operand& operator=(instruction_operand&& other) noexcept
		{
			indirection = other.indirection;
			operand = std::move(other.operand);

			return *this;
		}

		token operand;
        bool indirection;
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
		sprite_statement(token identifier_, std::vector<uint8_t> digits_)
			: base_statement(),
			  identifier(std::move(identifier_)),
			  digits(std::move(digits_))
		{}

		void accept(base_visitor& visitor) const override { return visitor.visit(*this); }

		[[nodiscard]] size_t source_line_beg() const override { return identifier.source_location.line; }
		[[nodiscard]] size_t source_line_end() const override { return identifier.source_location.line; }

		const token identifier;
		const std::vector<uint8_t> digits;
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

#endif //CCOMP_STATEMENTS_HPP
