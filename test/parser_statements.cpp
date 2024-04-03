#include <boost/test/unit_test.hpp>
#include <ccomp/parser.hpp>
#include <ccomp/lexer.hpp>

using namespace ccomp;

BOOST_AUTO_TEST_SUITE(parser_nodes)

	BOOST_AUTO_TEST_CASE(my_test)
	{
		auto lex = lexer(
				";; test program\n"
				"\n"
				"sprite my_sprite1 [0xFF, 0xFF, 0xFF, 0xFF]\n"
				";; constant definition\n"
				"define val 0b1010'1010 ;; holds value 170\n"
				"raw(val) ;; inline opcode\n"
				"\n"
				"\n"
				"proc my_function ;; declare and define a procedure\n"
				"    draw r1, r2, my_sprite1\n"
				"    ret\n"
				"endp my_function\n"
				";; entry point label -> .main:\n"
				".main:              \n"
				"	add r1,   val\n"
				"	sub r1,   r1\n"
				".exit:\n"
				"   raw(0000)\n"
				"   xor r0, r0\n"
				"   shr r0\n"
				"   cls\n"
		);

		auto parser = ccomp::parser(lex.enumerate_tokens());

		ast::abstract_tree tree = parser.make_tree();

		BOOST_CHECK_EQUAL(tree.branches().size(), 6);
	}

BOOST_AUTO_TEST_SUITE_END()
