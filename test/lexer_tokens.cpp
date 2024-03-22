#include <boost/test/unit_test.hpp>
#include <ccomp/lexer.hpp>


using namespace ccomp;


BOOST_AUTO_TEST_SUITE(lexer_tokens)

	BOOST_AUTO_TEST_CASE(check_token_types)
	{
		BOOST_TEST_MESSAGE("Checking the lexer separates the input stream and identifies tokens correctly");

		auto lex = lexer::from_buffer(
					"define val 0b1010'1010\n"
					"raw(val)\n"
					"proc my_function\n"
					"ret\n"
					"endp my_function\n"
					".main:\n"
					"add r1,val\n"
					"sub r1,r1"
				);

		const auto tokens = lex->enumerate_tokens();

		BOOST_CHECK_EQUAL(tokens.size(), 23);

		// Line 1
		BOOST_CHECK(tokens[0].type == token_type::keyword_define);
		BOOST_CHECK(tokens[1].type == token_type::identifier);
		BOOST_CHECK(tokens[2].type == token_type::numerical);

		// Line 2
		BOOST_CHECK(tokens[3].type == token_type::keyword_raw);
		BOOST_CHECK(tokens[4].type == token_type::parenthesis_open);
		BOOST_CHECK(tokens[5].type == token_type::identifier);
		BOOST_CHECK(tokens[6].type == token_type::parenthesis_close);

		// Line 3
		BOOST_CHECK(tokens[7].type == token_type::keyword_proc_start);
		BOOST_CHECK(tokens[8].type == token_type::identifier);

		// Line 4
		BOOST_CHECK(tokens[9].type == token_type::instruction);

		// Line 5
		BOOST_CHECK(tokens[10].type == token_type::keyword_proc_end);
		BOOST_CHECK(tokens[11].type == token_type::identifier);

		// Line 6
		BOOST_CHECK(tokens[12].type == token_type::dot);
		BOOST_CHECK(tokens[13].type == token_type::identifier);
		BOOST_CHECK(tokens[14].type == token_type::colon);

		// Line 7
		BOOST_CHECK(tokens[15].type == token_type::instruction);
		BOOST_CHECK(tokens[16].type == token_type::register_name);
		BOOST_CHECK(tokens[17].type == token_type::comma);
		BOOST_CHECK(tokens[18].type == token_type::identifier);

		// Line 8
		BOOST_CHECK(tokens[19].type == token_type::instruction);
		BOOST_CHECK(tokens[20].type == token_type::register_name);
		BOOST_CHECK(tokens[21].type == token_type::comma);
		BOOST_CHECK(tokens[22].type == token_type::register_name);
	}

	BOOST_AUTO_TEST_CASE(ignore_comments_and_whitespaces)
	{
		BOOST_TEST_MESSAGE("Checking the lexer ignores comments and handle whitespaces correctly");

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

		const auto tokens = lex->enumerate_tokens();

		BOOST_CHECK_EQUAL(tokens.size(), 23);

		// Line 1
		BOOST_CHECK(tokens[0].type == token_type::keyword_define);
		BOOST_CHECK(tokens[1].type == token_type::identifier);
		BOOST_CHECK(tokens[2].type == token_type::numerical);

		// Line 2
		BOOST_CHECK(tokens[3].type == token_type::keyword_raw);
		BOOST_CHECK(tokens[4].type == token_type::parenthesis_open);
		BOOST_CHECK(tokens[5].type == token_type::identifier);
		BOOST_CHECK(tokens[6].type == token_type::parenthesis_close);

		// Line 3
		BOOST_CHECK(tokens[7].type == token_type::keyword_proc_start);
		BOOST_CHECK(tokens[8].type == token_type::identifier);

		// Line 4
		BOOST_CHECK(tokens[9].type == token_type::instruction);

		// Line 5
		BOOST_CHECK(tokens[10].type == token_type::keyword_proc_end);
		BOOST_CHECK(tokens[11].type == token_type::identifier);

		// Line 6
		BOOST_CHECK(tokens[12].type == token_type::dot);
		BOOST_CHECK(tokens[13].type == token_type::identifier);
		BOOST_CHECK(tokens[14].type == token_type::colon);

		// Line 7
		BOOST_CHECK(tokens[15].type == token_type::instruction);
		BOOST_CHECK(tokens[16].type == token_type::register_name);
		BOOST_CHECK(tokens[17].type == token_type::comma);
		BOOST_CHECK(tokens[18].type == token_type::identifier);

		// Line 8
		BOOST_CHECK(tokens[19].type == token_type::instruction);
		BOOST_CHECK(tokens[20].type == token_type::register_name);
		BOOST_CHECK(tokens[21].type == token_type::comma);
		BOOST_CHECK(tokens[22].type == token_type::register_name);
	}

BOOST_AUTO_TEST_SUITE_END()
