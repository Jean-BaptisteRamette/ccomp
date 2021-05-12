#define BOOST_TEST_MODULE MainTest

#include <ccomp/lexer.hpp>
#include <ccomp/error.hpp>
#include <boost/test/unit_test.hpp>

using namespace ccomp;

namespace test
{
    template<typename LexemeValueType>
    bool token_values_equal(const std::vector<token> &seq1,
                            std::initializer_list<LexemeValueType> seq2)
    {
        if (seq1.size() != seq2.size())
            return false;

        auto it1 = seq1.begin();
        auto it2 = seq2.begin();

        for (; it1 != seq1.end() && it2 != seq2.end(); ++it1, ++it2)
            if (std::get<LexemeValueType>(it1->lexeme_value) != *it2)
                return false;

        return true;
    }

    bool token_types_equal(const std::vector<token> &seq1,
                           std::initializer_list<token_type> seq2)
    {
        if (seq1.size() != seq2.size())
            return false;

        auto it1 = seq1.begin();
        auto it2 = seq2.begin();

        for (; it1 != seq1.end() && it2 != seq2.end(); ++it1, ++it2)
            if (it1->type != *it2)
                return false;

        return true;
    }
}

using test_int_seq = std::initializer_list<std::size_t>;
using test_str_seq = std::initializer_list<std::string>;

BOOST_AUTO_TEST_CASE(lexer_numeric_values)
{
    using integer_sequence = std::initializer_list<std::size_t>;
    using string_sequence = std::initializer_list<std::string>;

    {
        BOOST_TEST_MESSAGE("[lexer_numeric_values]: Passing decimal tests");

        auto lexer = lexer::from_buff("980 00000345 0000 1234567890");

        const auto tokens = lexer.generate_tokens();
        const integer_sequence expected = { 980, 345, 0, 1234567890 };

        BOOST_REQUIRE(test::token_values_equal(tokens, expected));
        BOOST_TEST_MESSAGE("[lexer_numeric_values]: Decimal tests passed");
    }

    {
        BOOST_TEST_MESSAGE("[lexer_numeric_values]: Passing hexadecimal tests");

        auto lexer = lexer::from_buff("0x0 0x000000000000000000000 0xFF 0x100 0x535B0026 0x5bFaeBCD");
        const auto tokens { lexer.generate_tokens() };
        const integer_sequence expected = { 0, 0, 0xFF, 0x100, 0x535B0026, 0x5BFAEBCD };
        BOOST_REQUIRE(test::token_values_equal(tokens, expected));

        BOOST_TEST_MESSAGE("[lexer_numeric_values]: hexadecimal tests passed");
    }

    {
        BOOST_TEST_MESSAGE("[lexer_numeric_values]: Passing binary tests");

        auto lexer = lexer::from_buff("0b0 0b0000 0b10101010 0b1111'0000 0b1'0'1'1'1'1'1'1");
        const auto tokens { lexer.generate_tokens() };
        const integer_sequence expected = { 0, 0, 0b10101010, 0b1111'0000, 0b1'0'1'1'1'1'1'1 };
        BOOST_REQUIRE(test::token_values_equal(tokens, expected));

        BOOST_TEST_MESSAGE("[lexer_numeric_values]: binary tests passed");
    }

    {
        BOOST_TEST_MESSAGE("[lexer_numeric_values]: Passing octal tests");

        auto lexer = lexer::from_buff("0o234 0o0000 0o333143 0o666 0o767 0o5'55");
        const auto tokens { lexer.generate_tokens() };
        const integer_sequence expected = { 0234, 0, 0333143, 0666, 0767, 0555 };
        BOOST_REQUIRE(test::token_values_equal(tokens, expected));

        BOOST_TEST_MESSAGE("[lexer_numeric_values]: octal tests passed");
    }

    {
        auto lexer = lexer::from_buff("1''2");
        BOOST_REQUIRE_THROW(lexer.next_numeric_lexeme(), lexer_error);
    }

    {
        auto lexer = lexer::from_buff("");
        BOOST_REQUIRE_NO_THROW(lexer.next_numeric_lexeme());
    }

    {
        auto lexer = lexer::from_buff("0xFFAG");
        BOOST_REQUIRE_THROW(lexer.next_numeric_lexeme(), numeric_base_error);
    }

    {
        try
        {
            auto lexer = lexer::from_buff("0xFFAG");
            lexer.generate_tokens();
        } catch (const numeric_base_error& err)
        {
            BOOST_REQUIRE(err.base == 16 && err.digit == 'G');
        }
    }
}

BOOST_AUTO_TEST_CASE(lexer_str_lexeme)
{
    {
        auto lexer = lexer::from_buff("define   my_variable1");
        lexer.skip_next_ws(); BOOST_ASSERT(lexer.peek_chr() == 'd');
        BOOST_REQUIRE(lexer.next_str_lexeme() == "define");
        lexer.skip_next_ws(); BOOST_ASSERT(lexer.peek_chr() == 'm');
    }

    {
        /* TODO: Last error, ] is never read because we consume it when reading the number */
        auto lexer = lexer::from_buff("  [0x123BF ] [  123] [035]");
        const auto tokens { lexer.generate_tokens() };
        const auto expected = {
                token_type::angle_brackets_left, token_type::number, token_type::angle_brackets_right,
                token_type::angle_brackets_left, token_type::number, token_type::angle_brackets_right,
                token_type::angle_brackets_left, token_type::number, token_type::angle_brackets_right
        };

        BOOST_REQUIRE(test::token_types_equal(tokens, expected));
    }

    {
        auto lexer = lexer::from_buff("   define main, add :.   var____ [123] ");

        const auto expected = { token_type::define, token_type::main_symbol, token_type::comma,
                                token_type::instruction, token_type::colon, token_type::dot, token_type::identifier,
                                token_type::angle_brackets_left, token_type::number, token_type::angle_brackets_right };
        const auto tokens { lexer.generate_tokens() };
        BOOST_REQUIRE(test::token_types_equal(tokens, expected));
    }
}
