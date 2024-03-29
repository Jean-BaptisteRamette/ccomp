#include <boost/test/unit_test.hpp>
#include <ccomp/lexer.hpp>


using namespace ccomp;


BOOST_AUTO_TEST_SUITE(lexer_tokens)

	BOOST_AUTO_TEST_CASE(check_token_types)
	{
		BOOST_TEST_MESSAGE("Checking the lexer separates the input stream and identifies tokens correctly");

		auto lex = lexer(
					"define val 0b1010'1010\n"
					"raw(val)\n"
					"proc my_function\n"
					"ret\n"
					"endp my_function\n"
					".main:\n"
					"add r1,val\n"
					"sub r1,r1\n"
				);

		const auto tokens = lex.enumerate_tokens();
		const std::vector<token_type> expected = {
				token_type::keyword_define, token_type::identifier, token_type::numerical,
				token_type::keyword_raw, token_type::parenthesis_open, token_type::identifier, token_type::parenthesis_close,
				token_type::keyword_proc_start, token_type::identifier,
				token_type::instruction,
				token_type::keyword_proc_end, token_type::identifier,
				token_type::dot_label, token_type::identifier, token_type::colon,
				token_type::instruction, token_type::register_name, token_type::comma, token_type::identifier,
				token_type::instruction, token_type::register_name, token_type::comma, token_type::register_name
		};

		BOOST_CHECK_EQUAL(std::size(tokens), std::size(expected));

		for (size_t i = 0; i < std::size(expected); ++i)
			BOOST_CHECK(tokens[i].type == expected[i]);

	}

	BOOST_AUTO_TEST_CASE(ignore_comments_and_whitespaces)
	{
		BOOST_TEST_MESSAGE("Checking the lexer ignores comments and handle whitespaces correctly");

		auto lex = lexer(
				";; test program: source.c8\n"
				"\n"
				"sprite my_sprite [ 0xFF, 0b1111'0000, 0xEE ]\n"
				"\n"
				"define M1 10\n"
				"define M2 3\n"
				"\n"
				";; r1: multiplication operand 1\n"
				";; r2: multiplication operand 2\n"
				";; return: register r0\n"
				"proc multiply\n"
				"    xor r0, r0\n"
				"\n"
				"    sne r1, 0\n"
				"    jmp .done\n"
				"\n"
				".iter:\n"
				"    sne r2, 0\n"
				"    jmp .done\n"
				"\n"
				"    add r0, r1\n"
				"    dec r2\n"
				"\n"
				"    jmp .iter\n"
				"\n"
				".done:\n"
				"    ret\n"
				"endp multiply\n"
				"\n"
				"\n"
				".main:\n"
				"    mov r1, M1\n"
				"    mov r2, M2\n"
				"    call multiply\n"
				""
		);

		const auto tokens = lex.enumerate_tokens();

		const std::vector<token_type> expected = {
				token_type::keyword_sprite, token_type::identifier, token_type::bracket_open,
				token_type::numerical, token_type::comma, token_type::numerical, token_type::comma,
				token_type::numerical, token_type::bracket_close,
				token_type::keyword_define, token_type::identifier, token_type::numerical,
				token_type::keyword_define, token_type::identifier, token_type::numerical,
				token_type::keyword_proc_start, token_type::identifier,
				token_type::instruction, token_type::register_name, token_type::comma, token_type::register_name,
				token_type::instruction, token_type::register_name, token_type::comma, token_type::numerical,
				token_type::instruction, token_type::dot_label, token_type::identifier,
				token_type::dot_label, token_type::identifier, token_type::colon,
				token_type::instruction, token_type::register_name, token_type::comma, token_type::numerical,
				token_type::instruction, token_type::dot_label, token_type::identifier,
				token_type::instruction, token_type::register_name, token_type::comma, token_type::register_name,
				token_type::instruction, token_type::register_name,
				token_type::instruction, token_type::dot_label, token_type::identifier,
				token_type::dot_label, token_type::identifier, token_type::colon,
				token_type::instruction,
				token_type::keyword_proc_end, token_type::identifier,
				token_type::dot_label, token_type::identifier, token_type::colon,
				token_type::instruction, token_type::register_name, token_type::comma, token_type::identifier,
				token_type::instruction, token_type::register_name, token_type::comma, token_type::identifier,
				token_type::instruction, token_type::identifier,
		};

		BOOST_CHECK_EQUAL(std::size(tokens), std::size(expected));

		for (size_t i = 0; i < std::size(expected); ++i)
			BOOST_CHECK(tokens[i].type == expected[i]);
	}

BOOST_AUTO_TEST_SUITE_END()
