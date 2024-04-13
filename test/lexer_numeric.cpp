#include <boost/test/unit_test.hpp>
#include <chasm/lexer.hpp>


BOOST_AUTO_TEST_SUITE(lexer_numeric_constants)

	namespace details
	{
		uint16_t try_parse_int(std::string&& program)
		{
			auto lex = chasm::lexer(std::move(program));
			auto token = lex.enumerate_tokens()[0];

			return std::get<uint16_t>(token.data);
		}
	}

	BOOST_AUTO_TEST_CASE(comma_separated_digits)
	{
		BOOST_TEST_MESSAGE("checking comma separated digits syntax");

		BOOST_CHECK_EQUAL(details::try_parse_int("0xFF'FF"), 0xFFFF);
		BOOST_CHECK_EQUAL(details::try_parse_int("0xF'F'F'F"), 0xFFFF);
		BOOST_CHECK_EQUAL(details::try_parse_int("0b1111'1111'0000'0000"), 0b1111'1111'0000'0000);
		BOOST_CHECK_THROW(details::try_parse_int("0b1111'1111''0000'0000"), chasm::lexer_exception::undefined_character_token);
	}

	BOOST_AUTO_TEST_CASE(out_of_range_value)
	{
		BOOST_TEST_MESSAGE("checking out of range value detection");

		BOOST_CHECK_EQUAL(details::try_parse_int("65535"), 65535);
		BOOST_CHECK_THROW(details::try_parse_int("65536"), chasm::lexer_exception::numeric_constant_too_large);
	}

	BOOST_AUTO_TEST_CASE(invalid_base_digit)
	{
		BOOST_TEST_MESSAGE("checking invalid digits for numeric base");

		BOOST_CHECK_EQUAL(details::try_parse_int("0xABCD"), 0xABCD);
		BOOST_CHECK_THROW(details::try_parse_int("0xG"), chasm::lexer_exception::invalid_digit_for_base);
		BOOST_CHECK_THROW(details::try_parse_int("0b1111'2000"), chasm::lexer_exception::invalid_digit_for_base);
		BOOST_CHECK_THROW(details::try_parse_int("0o778"), chasm::lexer_exception::invalid_digit_for_base);

	}

	BOOST_AUTO_TEST_CASE(check_zero)
	{
		auto lex = chasm::lexer("0,0,0");
		BOOST_CHECK_NO_THROW(lex.enumerate_tokens());
	}

BOOST_AUTO_TEST_SUITE_END()
