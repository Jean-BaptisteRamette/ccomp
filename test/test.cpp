#define BOOST_TEST_MODULE ccomp_unittests
#include <boost/test/included/unit_test.hpp>
#include <ccomp/lexer.hpp>
#include <ccomp/parser.hpp>


using namespace ccomp;

BOOST_AUTO_TEST_CASE(lexer_peek_and_next)
{
    BOOST_TEST_MESSAGE("[lexer_tests]: peek() / next()");

    {
        auto lex = lexer::from_buffer("1337");
        BOOST_REQUIRE(lex->peek_chr() == '1');
        BOOST_REQUIRE(lex->next_chr() == '1');
        BOOST_REQUIRE(lex->peek_chr() == '3');
    }
}

BOOST_AUTO_TEST_CASE(lexer_eof_handling)
{
    BOOST_TEST_MESSAGE("[lexer_tests]: eof handling");

    {
        auto lex = lexer::from_buffer("1337");

        lex->next_chr();
        lex->next_chr();
        lex->next_chr();
        lex->next_chr();

        BOOST_REQUIRE(lex->istream.eof());
    }

    {
        auto lex = lexer::from_buffer("");

        BOOST_REQUIRE(lex->istream.eof());
    }
}

BOOST_AUTO_TEST_CASE(lexer_wspaces_skip)
{
    BOOST_TEST_MESSAGE("[lexer_tests]: whitespaces skip");

    {
        auto lex = lexer::from_buffer("1337");
        lex->skip_wspaces();

        BOOST_REQUIRE(lex->peek_chr() == '1');
    }

    {
        auto lex = lexer::from_buffer("       1337");
        lex->skip_wspaces();

        BOOST_REQUIRE(lex->peek_chr() == '1');
    }
}


BOOST_AUTO_TEST_CASE(lexer_comment_skip)
{
    BOOST_TEST_MESSAGE("[lexer_tests]: comment skip");

    {
        auto lex = lexer::from_buffer(";; this will get skipped\nx");
        
        lex->next_chr();
        lex->next_chr();
        
        lex->skip_comment();

        BOOST_REQUIRE(lex->next_chr() == 'x');
    }

    {
        auto lex = lexer::from_buffer(";; this will get skipped\n\nx");

        lex->next_chr();
        lex->next_chr();

        lex->skip_comment();

        BOOST_REQUIRE(lex->next_chr() == 'x');
    }

    {
        auto lex = lexer::from_buffer(";; skipped");
        lex->skip_comment();

        BOOST_REQUIRE(lex->istream.eof());
    }

    {
        auto lex = lexer::from_buffer("; not a proper comment");

        BOOST_REQUIRE_THROW(lex->next_token(), std::runtime_error);
    }
}

BOOST_AUTO_TEST_CASE(lexer_token_alpha)
{
    BOOST_TEST_MESSAGE("[lexer_tests]: alpha tokens");

    {
        auto lex = lexer::from_buffer("add sub whatever\nr0");

        BOOST_REQUIRE(lex->next_token().type == token_type::instruction);
        BOOST_REQUIRE(lex->next_token().type == token_type::instruction);
        BOOST_REQUIRE(lex->next_token().type == token_type::identifier);
        BOOST_REQUIRE(lex->next_token().type == token_type::register_name);
        BOOST_REQUIRE(lex->next_token().type == token_type::eof);
    }
}

BOOST_AUTO_TEST_CASE(lexer_token_numeric)
{
    BOOST_TEST_MESSAGE("[lexer_tests]: numeric tokens");

    {
        auto lex = lexer::from_buffer("   1000 00328032 0xDEAD'BEEF 0o777 0b1010'1010");

        BOOST_REQUIRE(lex->next_token().lexeme == "1000");
        BOOST_REQUIRE(lex->next_token().lexeme == "0328032");
        BOOST_REQUIRE(lex->next_token().lexeme == "xDEAD'BEEF");
        BOOST_REQUIRE(lex->next_token().lexeme == "o777");
        BOOST_REQUIRE(lex->next_token().lexeme == "b1010'1010");
    }

    {
        auto lex = lexer::from_buffer("1234567890'");
        BOOST_REQUIRE(lex->next_token().lexeme == "1234567890");
        BOOST_REQUIRE_THROW(lex->next_token(), ccomp::lexer_exception::undefined_token_error);
    }

    {
        auto lex = lexer::from_buffer("1234567890)");

        BOOST_REQUIRE(lex->next_token().lexeme == "1234567890");
        BOOST_REQUIRE(lex->next_token().type == token_type::parenthesis_close);
    }

    {
        auto lex = lexer::from_buffer("0b11'00'11''00");

        BOOST_REQUIRE(lex->next_token().lexeme == "b11'00'11");
        BOOST_REQUIRE_THROW(lex->next_token(), ccomp::lexer_exception::undefined_token_error);
    }

    {
        auto lex = lexer::from_buffer("134A56");

        BOOST_REQUIRE_THROW(lex->next_token(), lexer_exception::numeric_base_error);
    }
}


BOOST_AUTO_TEST_CASE(parser_node_tests)
{
    {
        constexpr auto code =
                "define numbera 0x77\n"
                "define numberb 0b11'10\n";

        auto parser = parser::from_buffer(code);

		const auto ast = parser->make_ast();

		BOOST_REQUIRE(ast.branches.size() == 2);
    }

	{
		constexpr auto code =
				"raw(255)\n"
				"raw(0b1111'0000)\n"
				"raw(test_opcode)\n";

		auto parser = parser::from_buffer(code);
		const auto ast = parser->make_ast();

		BOOST_REQUIRE(ast.branches.size() == 3);
	}

	{
		constexpr auto code =
				"raw(255)\n"
				"raw(0b1111'0000)\n"
				"raw(test)\n"
				"raw(define)\n";

		auto parser = parser::from_buffer(code);

		BOOST_REQUIRE_THROW(parser->make_ast(), parser_exception::expected_others_error);
	}

	{
		constexpr auto code = ".main:\n";
		auto parser = parser::from_buffer(code);
		const auto ast = parser->make_ast();

		BOOST_REQUIRE(ast.branches.size() == 1);
	}
}