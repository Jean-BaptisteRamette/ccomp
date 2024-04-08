#include <boost/test/unit_test.hpp>
#include <ccomp/lexer.hpp>
#include <ccomp/parser.hpp>
#include <ccomp/generator.hpp>


using namespace ccomp;

BOOST_AUTO_TEST_SUITE(code_generation)

	namespace details
	{
		std::vector<arch::opcode>
		try_codegen(std::string&& program)
		{
			auto lex = lexer(std::move(program));
			auto par = parser(lex.enumerate_tokens());
			auto ast = par.make_tree();

			return ast.generate();
		}
	}

	BOOST_AUTO_TEST_CASE(check_invalid_immediate_format)
	{
		BOOST_CHECK_THROW(details::try_codegen(".main:\n mov r0, 0x100"), generator_exception::invalid_immediate_format);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n mov ar, 0x1000"), generator_exception::invalid_immediate_format);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n jmp 0x1000"), generator_exception::invalid_immediate_format);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n jmp [0x1000]"), generator_exception::invalid_immediate_format);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n call 0x1000"), generator_exception::invalid_immediate_format);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n se r0, 0x100"), generator_exception::invalid_immediate_format);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n sne r0, 0x100"), generator_exception::invalid_immediate_format);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n add r0, 0x100"), generator_exception::invalid_immediate_format);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n rand r0, 0x100"), generator_exception::invalid_immediate_format);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n draw r0, r1, 0x10"), generator_exception::invalid_immediate_format);
	}

	BOOST_AUTO_TEST_CASE(check_invalid_operands_count)
	{
		BOOST_CHECK_THROW(details::try_codegen(".main:\n mov r0"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n mov r0, r1, r2"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n mov ar, 1, 2"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n mov ar, r1, r2"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n mov dt, r1, r2"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n mov r1, dt, 0"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n jmp"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n jmp @main, @main"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n jmp [100], [100]"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n call"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n ret 0"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n se r1, 0, 0"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n sne r1, 0, 0"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n add r1, r2, 1"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n sub r1"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n or"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n and 1"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n xor"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n rdump"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n rload"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n rand 0"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n bcd"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n wkey"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n ske"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n skne"), generator_exception::invalid_operands_count);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n cls 0"), generator_exception::invalid_operands_count);

		// > 3 operands
		BOOST_CHECK_THROW(details::try_codegen(".main:\n draw r0, r0, 1, 2"), assembler_error);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n shr 1, 2, 3, 4"), assembler_error);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n shl 1, 2, 3, 4"), assembler_error);
	}

	BOOST_AUTO_TEST_CASE(check_invalid_operand_type)
	{
		BOOST_CHECK_THROW(details::try_codegen(".main:\n mov r0, @main"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n mov r0, ar"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n mov ar, ar"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n mov dt, ar"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n mov r1, ar"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n jmp r0"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n call r0"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n call ar"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n se 0, 0"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n sne 0, 0"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n se 0, r0"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n sne 0, r0"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n add 0, r2"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n add 0, 1"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n sub 0, r1"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n sub r1, 0"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n or  r0, 1"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n and r0, 1"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n xor r0, 1"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n shr 1"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n shl 2"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n rdump 1"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n rload 1"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n rand 1, 0"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n bcd 1"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n wkey 1"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n ske 1"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n skne 1"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n draw r0, r0, r0"), generator_exception::invalid_operand_type);
	}

	BOOST_AUTO_TEST_CASE(check_raw_statements)
	{
		const auto code = details::try_codegen(".main:                     \n"
										       "    define opcode 0xFF'FF  \n"
										       "    raw(0x0000)            \n"
										       "    raw(1)                 \n"
										       "    raw(opcode)            \n"
										       "    raw(opcode)            \n");

		const auto expected_code = std::vector<arch::opcode>({
			0x0000,
			0x0001,
			0xFF'FF,
			0xFF'FF
		});

		BOOST_CHECK_EQUAL_COLLECTIONS(
				code.begin(),
				code.end(),
				expected_code.begin(),
				expected_code.end()
		);
	}

BOOST_AUTO_TEST_SUITE_END()
