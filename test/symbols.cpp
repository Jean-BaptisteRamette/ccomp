#include <boost/test/unit_test.hpp>
#include <chasm/lexer.hpp>
#include <chasm/parser.hpp>
#include <chasm/symbol_sanitizer.hpp>

#include "options_fixture.hpp"


using namespace chasm;


BOOST_FIXTURE_TEST_SUITE(symbol_sanitizer, options_fixture)

	namespace details
	{
		void try_assemble(std::string&& program)
		{
			auto lex = lexer(std::move(program));
			auto par = parser(lex.enumerate_tokens());
			auto ast = par.make_tree();
			ast.generate();
		}
	}

	BOOST_AUTO_TEST_CASE(check_undefined_jump_label_throws)
	{
		BOOST_CHECK_THROW(
			details::try_assemble(".main:         \n"
								  "    jmp @end   \n"),
			sanitize_exception::undefined_symbols
		);
	}

	BOOST_AUTO_TEST_CASE(check_undefined_procedure_call_throws)
	{
		BOOST_CHECK_THROW(
			details::try_assemble(".main:                \n"
						              "		call $undefined  \n"),
			sanitize_exception::undefined_symbols
		);
	}

	BOOST_AUTO_TEST_CASE(check_undefined_in_raw_statement_throws)
	{
		BOOST_CHECK_THROW(
			details::try_assemble(".main:            \n"
						          "    raw(val)      \n"),
			sanitize_exception::undefined_symbols
		);
	}

	BOOST_AUTO_TEST_CASE(check_pre_defined_symbol)
	{
		BOOST_CHECK_NO_THROW(
			details::try_assemble(".main:         \n"
							      "    jmp @end   \n"
							      ".end:          \n");
		);
	}

	BOOST_AUTO_TEST_CASE(check_in_procedure_pre_defined_symbol)
	{
		BOOST_CHECK_NO_THROW(
			details::try_assemble("proc tmp         \n"
					              ".start:          \n"
					              "    jmp @done    \n"
					              ".unreferenced:   \n"
					              ".done:           \n"
					              "    ret          \n"
					              "endp tmp         \n"
					              ".main:           \n"
					              "    call $tmp     \n")
		);
	}

	BOOST_AUTO_TEST_CASE(check_already_defined_throws)
	{
		BOOST_CHECK_THROW(
			details::try_assemble(".main:          \n"
								  "    jmp @main   \n"
								  ".main:          \n"),
			sanitize_exception::already_defined_symbol
		);
	}

	BOOST_AUTO_TEST_CASE(check_in_procedure_already_defined_throws)
	{
		BOOST_CHECK_THROW(
			details::try_assemble("proc tmp         \n"
						          ".start:          \n"
						          "    jmp @start   \n"
						          ".start:          \n"
						          "    ret          \n"
						          "endp tmp         \n"
						          ".main:           \n"
						          "    call $tmp     \n"),
			sanitize_exception::already_defined_symbol
		);
	}

	BOOST_AUTO_TEST_CASE(check_procedure_already_defined_throws)
	{
		BOOST_CHECK_THROW(
			details::try_assemble("proc tmp   \n"
						          "    ret    \n"
						          "endp tmp   \n"
						          "proc tmp   \n"
						          "    ret    \n"
						          "endp tmp   \n"
						          ".main:     \n"),
			sanitize_exception::already_defined_symbol
		);
	}

	BOOST_AUTO_TEST_CASE(check_scope_separation_proc_and_section)
	{
		BOOST_CHECK_NO_THROW(
			details::try_assemble("proc tmp       \n"
						          ".done:         \n"
						          "    ret        \n"
						          "endp tmp       \n"
						          "               \n"
						          ".main:         \n"
						          "    jmp @done  \n"
						          ".done:         \n")
		);
	}

	BOOST_AUTO_TEST_CASE(check_scope_separation_proc_and_proc)
	{
		BOOST_CHECK_NO_THROW(
			details::try_assemble("proc tmp   \n"
						          ".done:     \n"
						          "    ret    \n"
						          "endp tmp   \n"
						          "           \n"
						          "proc tmp2  \n"
						          ".done:     \n"
						          "    ret    \n"
						          "endp tmp2  \n"
						          "           \n"
						          ".main:     \n")
		);
	}

	BOOST_AUTO_TEST_CASE(check_main_label_defined_at_scope0)
	{
		BOOST_CHECK_THROW(
			details::try_assemble(".not_main:"),
			assembler_error
		);
	}

	BOOST_AUTO_TEST_CASE(check_raw_not_scope0)
	{
		BOOST_CHECK_THROW(
			details::try_assemble("raw(0xDEAD)  \n"
						          ".main:       \n"),
			assembler_error
		);
	}

	BOOST_AUTO_TEST_CASE(check_constant_symbols_nothrow)
	{
		BOOST_CHECK_NO_THROW(
			details::try_assemble("proc f             \n"
						          "    define C1 10   \n"
						          ".begin_proc:       \n"
						          "    define C2 10   \n"
						          "    mov r1, C1     \n"
						          "                   \n"
						          ".end_proc:         \n"
						          "    define C2 11   \n"
						          "    mov r1, C2     \n"
						          "    ret            \n"
						          "endp f             \n"
						          "                   \n"
						          "proc g             \n"
						          "    define C1 5    \n"
						          "    ret            \n"
						          "endp g             \n"
						          "                   \n"
						          ".main:             \n"
						          "                   \n")
		);
	}

	BOOST_AUTO_TEST_CASE(check_constant_symbols_throws)
	{
		BOOST_CHECK_THROW(
			details::try_assemble("proc f             \n"
						          ".begin_proc:       \n"
						          "    define C2 10   \n"
						          "    mov r1, C1     \n"
						          ".C1:               \n"
						          "    ret            \n"
						          "endp f             \n"
						          "                   \n"
						          "proc g             \n"
						          "    define C1 5    \n"
						          "    ret            \n"
						          "endp g             \n"
						          "                   \n"
						          ".main:             \n"
						          "                   \n"),
			sanitize_exception::undefined_symbols
		);
	}

	BOOST_AUTO_TEST_CASE(inter_procedure_call_to_defined)
	{
		BOOST_CHECK_NO_THROW(
			details::try_assemble("proc fb         \n"
						          "    ret         \n"
						          "endp fb         \n"
						          "                \n"
						          "proc fa         \n"
						          "    call $fb    \n"
						          "    ret         \n"
						          "endp fa         \n"
						          "                \n"
						          ".main:          \n")
		);
	}

	BOOST_AUTO_TEST_CASE(inter_procedure_call_to_yet_undefined)
	{
		BOOST_CHECK_NO_THROW(
			details::try_assemble("proc fa         \n"
						          "    call $fb    \n"
						          "    ret         \n"
						          "endp fa         \n"
						          "                \n"
						          "proc fb         \n"
						          "    ret         \n"
						          "endp fb         \n"
						          "                \n"
						          ".main:          \n")
		);
	}

	BOOST_AUTO_TEST_CASE(inter_procedure_call_to_never_defined)
	{
		BOOST_CHECK_THROW(
			details::try_assemble("proc fa           \n"
						          "    call $fb      \n"
						          "    ret           \n"
						          "endp fa           \n"
						          "                  \n"
						          "proc fd           \n"
						          "    ret           \n"
						          "endp fd           \n"
						          "                  \n"
						          ".main:            \n"),
			sanitize_exception::undefined_symbols
		);
	}

	BOOST_AUTO_TEST_CASE(check_sprite_already_defined)
	{
		BOOST_CHECK_THROW(
			details::try_assemble("sprite my_sprite1 [0xA, 0xB, 0xC, 0xD]\n"
			                      "sprite my_sprite1 [0xA, 0xB, 0xC, 0xD]\n"
			                      ".main:            \n"),
			sanitize_exception::already_defined_symbol
		);
	}

	BOOST_AUTO_TEST_CASE(check_undefined_sprite)
	{
		BOOST_CHECK_THROW(
			details::try_assemble(".main:                        \n"
			                      "    draw r0, r0, #my_sprite   \n"),
			sanitize_exception::undefined_symbols
		);
	}

	BOOST_AUTO_TEST_CASE(check_sprite_is_global)
	{
		BOOST_CHECK_THROW(
			details::try_assemble(".main:                \n"
			                      "    sprite s [0xA]    \n"),
			assembler_error
		);
	}

BOOST_AUTO_TEST_SUITE_END()
