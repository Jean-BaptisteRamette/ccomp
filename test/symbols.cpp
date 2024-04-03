#include <boost/test/unit_test.hpp>
#include <ccomp/lexer.hpp>
#include <ccomp/parser.hpp>
#include <ccomp/symbol_sanitizer.hpp>


using namespace ccomp;


BOOST_AUTO_TEST_SUITE(symbol_sanitizer)

	BOOST_AUTO_TEST_CASE(check_undefined_jump_label_throws)
	{
		auto lex = lexer(".main:\n"
						 "    jmp @exit");

		auto p = parser(lex.enumerate_tokens());
		const auto ast = p.make_tree();
		BOOST_CHECK_THROW(ast.generate(), sanitize_exception::undefined_symbols);
	}

	BOOST_AUTO_TEST_CASE(check_undefined_procedure_call_throws)
	{
		auto lex = lexer(".main:                \n"
						 "		call undefined  \n");

		auto p = parser(lex.enumerate_tokens());
		const auto ast = p.make_tree();
		BOOST_CHECK_THROW(ast.generate(), sanitize_exception::undefined_symbols);
	}

	BOOST_AUTO_TEST_CASE(check_undefined_in_raw_statement_throws)
	{
		auto lex = lexer(".main:            \n"
						 "    raw(val)      \n");

		auto p = parser(lex.enumerate_tokens());
		const auto ast = p.make_tree();
		BOOST_CHECK_THROW(ast.generate(), sanitize_exception::undefined_symbols);
	}

	BOOST_AUTO_TEST_CASE(check_pre_defined_symbol)
	{
		auto lex = lexer(".main:         \n"
						 "    jmp @exit  \n"
						 ".exit:         \n");

		auto p = parser(lex.enumerate_tokens());
		const auto ast = p.make_tree();
		BOOST_CHECK_NO_THROW(ast.generate());
	}

	BOOST_AUTO_TEST_CASE(check_in_procedure_pre_defined_symbol)
	{
		auto lex = lexer("proc tmp         \n"
						 ".start:          \n"
						 "    jmp @done    \n"
						 ".unreferenced:   \n"
						 ".done:           \n"
						 "    ret          \n"
						 "endp tmp         \n"
						 ".main:           \n"
						 "    call tmp     \n");

		auto p = parser(lex.enumerate_tokens());
		const auto ast = p.make_tree();
		BOOST_CHECK_NO_THROW(ast.generate());
	}

	BOOST_AUTO_TEST_CASE(check_already_defined_throws)
	{
		auto lex = lexer(".main:          \n"
						 "    jmp @main   \n"
						 ".main:          \n");

		auto p = parser(lex.enumerate_tokens());
		const auto ast = p.make_tree();
		BOOST_CHECK_THROW(ast.generate(), sanitize_exception::already_defined_symbol);
	}

	BOOST_AUTO_TEST_CASE(check_in_procedure_already_defined_throws)
	{
		auto lex = lexer("proc tmp         \n"
						 ".start:          \n"
						 "    jmp @start   \n"
						 ".start:          \n"
						 "    ret          \n"
						 "endp tmp         \n"
						 ".main:           \n"
						 "    call tmp     \n");

		auto p = parser(lex.enumerate_tokens());
		const auto ast = p.make_tree();
		BOOST_CHECK_THROW(ast.generate(), sanitize_exception::already_defined_symbol);
	}

	BOOST_AUTO_TEST_CASE(check_procedure_already_defined_throws)
	{
		auto lex = lexer("proc tmp   \n"
						 "    ret    \n"
						 "endp tmp   \n"
						 "proc tmp   \n"
						 "    ret    \n"
						 "endp tmp   \n"
						 ".main:     \n");

		auto p = parser(lex.enumerate_tokens());
		const auto ast = p.make_tree();
		BOOST_CHECK_THROW(ast.generate(), sanitize_exception::already_defined_symbol);
	}

	BOOST_AUTO_TEST_CASE(check_scope_separation_proc_and_section)
	{
		auto lex = lexer("proc tmp       \n"
						 ".done:         \n"
						 "    ret        \n"
						 "endp tmp       \n"
						 "               \n"
						 ".main:         \n"
						 "    jmp @done  \n"
						 ".done:         \n");

		auto p = parser(lex.enumerate_tokens());
		const auto ast = p.make_tree();
		BOOST_CHECK_NO_THROW(ast.generate());
	}

	BOOST_AUTO_TEST_CASE(check_scope_separation_proc_and_proc)
	{
		auto lex = lexer("proc tmp   \n"
						 ".done:     \n"
						 "    ret    \n"
						 "endp tmp   \n"
						 "           \n"
						 "proc tmp2  \n"
						 ".done:     \n"
						 "    ret    \n"
						 "endp tmp2  \n"
						 "           \n"
						 ".main:     \n");

		auto p = parser(lex.enumerate_tokens());
		const auto ast = p.make_tree();
		BOOST_CHECK_NO_THROW(ast.generate());
	}

	BOOST_AUTO_TEST_CASE(check_main_label_defined_at_scope0)
	{
		auto lex = lexer(".not_main:");
		auto p = parser(lex.enumerate_tokens());
		const auto ast = p.make_tree();
		BOOST_CHECK_THROW(ast.generate(), assembler_error);
	}

	BOOST_AUTO_TEST_CASE(check_constant_symbols_nothrow)
	{
		auto lex = lexer("proc f             \n"
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
						 "                   \n");

		auto p = parser(lex.enumerate_tokens());
		const auto ast = p.make_tree();
		BOOST_CHECK_NO_THROW(ast.generate());
	}

	BOOST_AUTO_TEST_CASE(check_constant_symbols_throws)
	{
		auto lex = lexer("proc f             \n"
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
						 "                   \n");

		auto p = parser(lex.enumerate_tokens());
		const auto ast = p.make_tree();
		BOOST_CHECK_THROW(ast.generate(), sanitize_exception::undefined_symbols);
	}

	BOOST_AUTO_TEST_CASE(inter_procedure_call_to_defined)
	{
		auto lex = lexer("proc fb\n"
						 "    ret\n"
						 "endp fb\n"
						 "\n"
						 "proc fa\n"
						 "    call fb\n"
						 "    ret\n"
						 "endp fa\n"
						 "\n"
						 ".main:\n");

		auto p = parser(lex.enumerate_tokens());
		const auto ast = p.make_tree();

		BOOST_CHECK_NO_THROW(ast.generate());
	}

	BOOST_AUTO_TEST_CASE(inter_procedure_call_to_yet_undefined)
	{
		auto lex = lexer("proc fa\n"
						 "    call fb\n"
						 "    ret\n"
						 "endp fa\n"
						 "\n"
						 "proc fb\n"
						 "    ret\n"
						 "endp fb\n"
						 "\n"
						 ".main:\n");

		auto p = parser(lex.enumerate_tokens());
		const auto ast = p.make_tree();

		BOOST_CHECK_NO_THROW(ast.generate());
	}

	BOOST_AUTO_TEST_CASE(inter_procedure_call_to_never_defined)
	{
		auto lex = lexer("proc fa\n"
						 "    call fb\n"
						 "    ret\n"
						 "endp fa\n"
						 "\n"
						 "proc fd\n"
						 "    ret\n"
						 "endp fd\n"
						 "\n"
						 ".main:\n");

		auto p = parser(lex.enumerate_tokens());
		const auto ast = p.make_tree();

		BOOST_CHECK_THROW(ast.generate(), sanitize_exception::undefined_symbols);
	}

BOOST_AUTO_TEST_SUITE_END()
