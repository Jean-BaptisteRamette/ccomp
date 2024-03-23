#ifndef CCOMP_STATEMENTS_HPP
#define CCOMP_STATEMENTS_HPP

#include <ccomp/lexer.hpp>
#include <vector>


namespace ccomp::ast
{
	struct base_statement;

    struct procedure_statement;
    struct operand;
    struct instruction_statement;
    struct define_statement;
    struct raw_statement;
	struct label_statement;

	using statement = std::unique_ptr<base_statement>;

    struct base_statement
    {
		base_statement() = default;
		base_statement(const base_statement&) = delete;
		base_statement(base_statement&&) = delete;
		base_statement& operator=(const base_statement&) = delete;
		base_statement& operator=(base_statement&&) = delete;

		CCOMP_NODISCARD virtual source_location source_begin() const = 0;
		CCOMP_NODISCARD virtual source_location source_end()   const = 0;

        virtual ~base_statement() = default;
	};

    struct procedure_statement : base_statement
    {
        procedure_statement(std::string name_, std::vector<statement> inner_statements_)
            : name(std::move(name_)),
              inner_statements(std::move(inner_statements_))
        {}

		CCOMP_NODISCARD
		source_location source_begin() const override
		{
			return {};
		}

		CCOMP_NODISCARD
		source_location source_end() const override
		{
			return {};
		}

        std::string name;

		// raw, define, instructions and label statements
        std::vector<statement> inner_statements;
    };

    struct instruction_operand
    {
		// TODO: ATM this class does not need to inherit from base_statement

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

        bool indirection;

		token operand;
    };

    struct instruction_statement : base_statement
    {
		instruction_statement(token mnemonic_, std::vector<instruction_operand> operands_)
			: mnemonic(std::move(mnemonic_)),
			  operands(std::move(operands_))
		{}

		CCOMP_NODISCARD source_location source_begin() const override {  return {}; }
		CCOMP_NODISCARD source_location source_end()   const override {  return {}; }

    	token mnemonic;

		// chip-8 instructions 0 to 3 operands
		std::vector<instruction_operand> operands;
    };

    struct define_statement : base_statement
    {
        define_statement(token identifier_, token value_)
            : base_statement(),
			  identifier(std::move(identifier_)),
			  value(std::move(value_))
        {}

		CCOMP_NODISCARD source_location source_begin() const override {  return {}; }
		CCOMP_NODISCARD source_location source_end()   const override {  return {}; }

        token identifier;
        token value;
    };

    struct raw_statement : base_statement
    {
		explicit raw_statement(token opcode_)
			: base_statement(),
			  opcode(std::move(opcode_))
		{}

		CCOMP_NODISCARD source_location source_begin() const override {  return {}; }
		CCOMP_NODISCARD source_location source_end()   const override {  return {}; }

        token opcode;
    };

	struct label_statement : base_statement
	{
		explicit label_statement(token identifier_)
			: base_statement(),
			  identifier_(std::move(identifier_))
		{}

		CCOMP_NODISCARD source_location source_begin() const override {  return {}; }
		CCOMP_NODISCARD source_location source_end()   const override {  return {}; }

		token identifier_;
	};
}

#endif //CCOMP_STATEMENTS_HPP
