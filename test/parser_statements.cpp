#include <boost/test/unit_test.hpp>
#include <ccomp/parser.hpp>
#include <ccomp/lexer.hpp>

using namespace ccomp;

BOOST_AUTO_TEST_SUITE(parser_nodes)

	BOOST_AUTO_TEST_CASE(my_test)
	{
		auto lex = lexer::from_buffer(
				";; test program\n"
				"\n"
				";; constant definition\n"
				"define val 0b1010'1010 ;; holds value 170\n"
				"raw(val) ;; inline opcode\n"
				"\n"
				"\n"
				"proc my_function ;; declare and define a procedure\n"
				"    ret\n"
				"endp my_function\n"
				";; entry point label -> .main:\n"
				".main:              \n"
				"	add r1,   val\n"
				"	sub r1,   r1"
		);

		auto parser = ccomp::parser(lex->enumerate_tokens());

		ast::abstract_tree tree = parser.make_tree();

		BOOST_CHECK_EQUAL(tree.branches.size(), 6);
	}

BOOST_AUTO_TEST_SUITE_END()
