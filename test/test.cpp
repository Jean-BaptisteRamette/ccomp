#define BOOST_TEST_MODULE ccomp_unittests
#include <boost/test/included/unit_test.hpp>
#include <ccomp/lexer.hpp>


using namespace ccomp;


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
        auto lex = lexer::from_buff(";; skipped\n\nx");

        lex->next_chr();
        lex->next_chr();

        lex->skip_comment();

        BOOST_REQUIRE(lex->next_chr() == '\n');
    }

    {
        auto lex = lexer::from_buff(";; skipped");
        BOOST_REQUIRE(lex->next_token().type == token_type::eof);
    }
}