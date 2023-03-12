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

BOOST_AUTO_TEST_CASE(lexer_token_type)
{
    BOOST_TEST_MESSAGE("[lexer_tests]: token type");

    {
        auto lex = lexer::from_buff("add sub whatever\nr0");

        BOOST_REQUIRE(lex->next_token().type == token_type::instruction);
        BOOST_REQUIRE(lex->next_token().type == token_type::instruction);
        BOOST_REQUIRE(lex->next_token().type == token_type::identifier);
        BOOST_REQUIRE(lex->next_token().type == token_type::gp_register);
    }
}