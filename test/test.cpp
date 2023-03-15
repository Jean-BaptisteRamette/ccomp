#define BOOST_TEST_MODULE ccomp_unittests
#include <boost/test/included/unit_test.hpp>
#include <ccomp/lexer.hpp>


using namespace ccomp;

BOOST_AUTO_TEST_CASE(lexer_peek_and_next)
{
    BOOST_TEST_MESSAGE("[lexer_tests]: peek() / next()");

    {
        auto lex = lexer::from_buff("1337");
        BOOST_REQUIRE(lex->peek_chr() == '1');
        BOOST_REQUIRE(lex->next_chr() == '1');
        BOOST_REQUIRE(lex->peek_chr() == '3');
    }
}

BOOST_AUTO_TEST_CASE(lexer_eof_handling)
{
    BOOST_TEST_MESSAGE("[lexer_tests]: eof handling");

    {
        auto lex = lexer::from_buff("1337");

        lex->next_chr();
        lex->next_chr();
        lex->next_chr();
        lex->next_chr();

        BOOST_REQUIRE(lex->istream.eof());
    }

    {
        auto lex = lexer::from_buff("");

        BOOST_REQUIRE(lex->istream.eof());
    }
}

BOOST_AUTO_TEST_CASE(lexer_wspaces_skip)
{
    BOOST_TEST_MESSAGE("[lexer_tests]: whitespaces skip");

    {
        auto lex = lexer::from_buff("1337");
        lex->skip_wspaces();

        BOOST_REQUIRE(lex->peek_chr() == '1');
    }

    {
        auto lex = lexer::from_buff("       1337");
        lex->skip_wspaces();

        BOOST_REQUIRE(lex->peek_chr() == '1');
    }
}


BOOST_AUTO_TEST_CASE(lexer_comment_skip)
{
    BOOST_TEST_MESSAGE("[lexer_tests]: comment skip");

    {
        auto lex = lexer::from_buff(";; this will get skipped\nx");
        
        lex->next_chr();
        lex->next_chr();
        
        lex->skip_comment();

        BOOST_REQUIRE(lex->next_chr() == 'x');
    }

    {
        auto lex = lexer::from_buff(";; this will get skipped\n\nx");

        lex->next_chr();
        lex->next_chr();

        lex->skip_comment();

        BOOST_REQUIRE(lex->next_chr() == 'x');
    }

    {
        auto lex = lexer::from_buff(";; skipped");
        lex->skip_comment();

        BOOST_REQUIRE(lex->istream.eof());
    }

    {
        auto lex = lexer::from_buff("; not a proper comment");

        BOOST_REQUIRE(lex->next_token().type == token_type::undefined);
    }
}

BOOST_AUTO_TEST_CASE(lexer_token_alpha)
{
    BOOST_TEST_MESSAGE("[lexer_tests]: alpha tokens");

    {
        auto lex = lexer::from_buff("add sub whatever\nr0");

        BOOST_REQUIRE(lex->next_token().type == token_type::instruction);
        BOOST_REQUIRE(lex->next_token().type == token_type::instruction);
        BOOST_REQUIRE(lex->next_token().type == token_type::identifier);
        BOOST_REQUIRE(lex->next_token().type == token_type::gp_register);
        BOOST_REQUIRE(lex->next_token().type == token_type::eof);
    }
}

BOOST_AUTO_TEST_CASE(lexer_token_numeric)
{
    BOOST_TEST_MESSAGE("[lexer_tests]: numeric tokens");

    {
        auto lex = lexer::from_buff("   1000 00328032 0xDEAD'BEEF 0o777 0b1010'1010");

        BOOST_REQUIRE(lex->next_token().lexeme == "1000");
        BOOST_REQUIRE(lex->next_token().lexeme == "0328032");
        BOOST_REQUIRE(lex->next_token().lexeme == "xDEAD'BEEF");
        BOOST_REQUIRE(lex->next_token().lexeme == "o777");
        BOOST_REQUIRE(lex->next_token().lexeme == "b1010'1010");
    }

    {
        auto lex = lexer::from_buff("1234567890'");
        BOOST_REQUIRE(lex->next_token().lexeme == "1234567890");
        BOOST_REQUIRE(lex->next_token().type == token_type::undefined);
    }

    {
        auto lex = lexer::from_buff("0b11'00'11''00");

        BOOST_REQUIRE(lex->next_token().lexeme == "b11'00'11");
        BOOST_REQUIRE(lex->next_token().type == token_type::undefined);
    }

    {
        auto lex = lexer::from_buff("134A56");

        BOOST_REQUIRE_THROW(lex->next_token(), lexer_exception::numeric_base_error);
    }
}