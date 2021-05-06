#define BOOST_TEST_MODULE MainTest

#include <ccomp/lexer.hpp>
#include <boost/test/unit_test.hpp>

using namespace ccomp;

template<typename LexemeValueType>
bool lexeme_values_equal(const std::vector<token>& seq1,
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

BOOST_AUTO_TEST_CASE(lexer_numeric_values)
{
    using integer_sequence = std::initializer_list<std::size_t>;
    using string_sequence = std::initializer_list<std::string>;

    {
        BOOST_TEST_MESSAGE("[lexer_numeric_values]: Passing decimal tests");

        auto lexer = lexer::from_buff("980 00000345 0000 1234567890");

        const auto tokens = lexer.generate_tokens();
        const integer_sequence expected = { 980, 345, 0, 1234567890 };

        BOOST_REQUIRE(lexeme_values_equal(tokens, expected));
        BOOST_TEST_MESSAGE("[lexer_numeric_values]: Decimal tests passed");
    }

    {
        BOOST_TEST_MESSAGE("[lexer_numeric_values]: Passing hexadecimal tests");

        auto lexer = lexer::from_buff("0x0 0x000000000000000000000 0xFF 0x100 0x535B0026 0x5bFaeBCD");
        const auto tokens { lexer.generate_tokens() };
        const integer_sequence expected = { 0, 0, 0xFF, 0x100, 0x535B0026, 0x5BFAEBCD };
        BOOST_REQUIRE(lexeme_values_equal(tokens, expected));

        BOOST_TEST_MESSAGE("[lexer_numeric_values]: hexadecimal tests passed");
    }

    {
        BOOST_TEST_MESSAGE("[lexer_numeric_values]: Passing binary tests");

        auto lexer = lexer::from_buff("0b0 0b0000 0b10101010 0b1111'0000 0b1'0'1'1'1'1'1'1");
        const auto tokens { lexer.generate_tokens() };
        const integer_sequence expected = { 0, 0, 0b10101010, 0b1111'0000, 0b1'0'1'1'1'1'1'1 };
        BOOST_REQUIRE(lexeme_values_equal(tokens, expected));

        BOOST_TEST_MESSAGE("[lexer_numeric_values]: binary tests passed");
    }

    {
        BOOST_TEST_MESSAGE("[lexer_numeric_values]: Passing octal tests");

        auto lexer = lexer::from_buff("0o234 0o0000 0o333143 0o666 0o767 0o555");
        const auto tokens { lexer.generate_tokens() };
        const integer_sequence expected = { 0234, 0, 0333143, 0666, 0767, 0555 };
        BOOST_REQUIRE(lexeme_values_equal(tokens, expected));

        BOOST_TEST_MESSAGE("[lexer_numeric_values]: octal tests passed");
    }

    {
        auto lexer = lexer::from_buff("");
        BOOST_REQUIRE_NO_THROW(lexer.next_numeric_lexeme());
    }

    {
        auto lexer = lexer::from_buff("0xFFAG");
        BOOST_REQUIRE_THROW(lexer.next_numeric_lexeme(), std::runtime_error);
    }
}

