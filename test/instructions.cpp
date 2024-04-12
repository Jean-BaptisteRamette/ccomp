#include <boost/test/unit_test.hpp>
#include <chasm/lexer.hpp>
#include <chasm/parser.hpp>
#include <chasm/generator.hpp>


BOOST_AUTO_TEST_SUITE(instruction_operands)

	using namespace chasm;

	namespace details
	{
		void try_codegen(std::string&& program)
		{
			auto lex = lexer(std::move(program));
			auto par = parser(lex.enumerate_tokens());
			auto ast = par.make_tree();

			ast.generate();
		}
	}

	BOOST_AUTO_TEST_CASE(check_invalid_immediate_format)
	{
		BOOST_CHECK_THROW(details::try_codegen(".main:\n mov r0, 0x100"), generator_exception::invalid_immediate_format);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n jmp [0x1000]"), generator_exception::invalid_immediate_format);
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

	BOOST_AUTO_TEST_CASE(check_address_operands)
	{
		//
		// User should be able to mov into AR (I) register a sprite/label/function or any address as
		// he may want to refer to a sprite stored in the interpreter's memory such as font characters
		//
		BOOST_CHECK_NO_THROW(details::try_codegen(".main:\n mov ar, @main"));
		BOOST_CHECK_NO_THROW(
			details::try_codegen("sprite s [0xA, 0xA]  \n"
								 ".main:               \n"
								 "    mov ar, #s       \n")
		);

		BOOST_CHECK_NO_THROW(
			details::try_codegen("proc f          \n"
			                     "    ret         \n"
			                     "endp f          \n"
			                     "                \n"
			                     ".main:          \n"
			                     "    mov ar, $f  \n")
		);

		BOOST_CHECK_NO_THROW(
			details::try_codegen(".main:             \n"
								 "    mov ar, 0xFFF \n")
		);

		BOOST_CHECK_NO_THROW(details::try_codegen(".main:\n jmp @main"));

		BOOST_CHECK_NO_THROW(
			details::try_codegen("proc f      \n"
			                     "    ret     \n"
			                     "endp f      \n"
			                     "            \n"
			                     ".main:      \n"
			                     "    call $f \n")
		);

		BOOST_CHECK_THROW(
			details::try_codegen(".main:\n "
			                     "    call @main"),
			generator_exception::invalid_operand_type
		);

		// If the user wants to do this, he must use the raw() statement
		BOOST_CHECK_THROW(details::try_codegen(".main:\n jmp 0x0000"), generator_exception::invalid_operand_type);
		BOOST_CHECK_THROW(details::try_codegen(".main:\n call 0"), generator_exception::invalid_operand_type);
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

BOOST_AUTO_TEST_SUITE_END()
