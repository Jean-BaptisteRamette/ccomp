#include <boost/test/unit_test.hpp>
#include <ccomp/lexer.hpp>

using namespace ccomp;


BOOST_AUTO_TEST_SUITE(lexer_numeric_constants)

BOOST_AUTO_TEST_CASE(comma_separated_digits)
{
	BOOST_TEST_MESSAGE("checking comma separated digits syntax");

	{
		auto lex = lexer::from_buffer("0xFF'FF");
		BOOST_CHECK_EQUAL(lex->read_numeric_lexeme(), 0xFFFF);
	}

	{
		auto lex = lexer::from_buffer("0xF'F'F'F");
		BOOST_CHECK_EQUAL(lex->read_numeric_lexeme(), 0xFFFF);
	}

	{
		auto lex = lexer::from_buffer("0b1111'1111'0000'0000");
		BOOST_CHECK_EQUAL(lex->read_numeric_lexeme(), 0xFF00);
	}

	{
		auto lex = lexer::from_buffer("0b1111'1111''0000'0000");
		BOOST_CHECK_EQUAL(lex->read_numeric_lexeme(), 0xFF);
	}
}

BOOST_AUTO_TEST_CASE(out_of_range_value)
{
	BOOST_TEST_MESSAGE("checking out of range value detection");

	{
		auto lex = lexer::from_buffer("65535");
		BOOST_CHECK_EQUAL(lex->read_numeric_lexeme(), 65535);
	}

	{
		auto lex = lexer::from_buffer("65536");
		BOOST_CHECK_THROW(lex->read_numeric_lexeme(), lexer_exception::numeric_constant_too_large);
	}
}

BOOST_AUTO_TEST_CASE(invalid_base_digit)
{
	BOOST_TEST_MESSAGE("checking invalid digits for numeric base");

	{
		auto lex = lexer::from_buffer("0xABCD");
		BOOST_CHECK_EQUAL(lex->read_numeric_lexeme(), 0xABCD);
	}

	{
		auto lex = lexer::from_buffer("ABCDE");
		BOOST_CHECK_THROW(lex->read_numeric_lexeme(), lexer_exception::invalid_digit_for_base);
	}

	{
		auto lex = lexer::from_buffer("0b1111'2000");
		BOOST_CHECK_THROW(lex->read_numeric_lexeme(), lexer_exception::invalid_digit_for_base);
	}

	{
		auto lex = lexer::from_buffer("0o778");
		BOOST_CHECK_THROW(lex->read_numeric_lexeme(), lexer_exception::invalid_digit_for_base);
	}
}


BOOST_AUTO_TEST_SUITE_END()
