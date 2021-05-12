#include <fstream>
#include <iostream>
#include <ccomp/lexer.hpp>
#include <ccomp/error.hpp>

static constexpr int BASE_BIN { 2 };
static constexpr int BASE_OCT { 8 };
static constexpr int BASE_DEC { 10 };
static constexpr int BASE_HEX { 16 };

namespace
{
    template<typename Map>
    concept mapping_type = requires { typename Map::mapped_type; };

    template<mapping_type Map, typename Key>
    typename Map::mapped_type map_value_or(
            const Map& map,
            const Key& key,
            const typename Map::mapped_type& default_value)
    {
        /*!
         * @brief return the value mapped to the key if found, else default value
         */
        const auto pos { map.find(key) };

        if (pos == std::end(map))
            return default_value;

        return pos->second;
    }

    bool is_valid_digit(char digit, int base)
    {
        /*!
         * @brief checks if the input digit is a valid digit for the given base
         */
        if (base == BASE_BIN && (digit == '0' || digit == '1'))
            return true;

        if (base == BASE_OCT && digit >= '0' && digit <= '7')
            return true;

        if (base == BASE_DEC && digit >= '0' && digit <= '9')
            return true;

        const int lower { std::tolower(digit) };
        if (base == BASE_HEX && (lower >= '0' && lower <= '9') || (lower >= 'a' && lower <= 'f'))
            return true;

        return false;
    }

    uint8_t digit_to_value(char c)
    {
        /*!
         * @brief returns a digit's value.
         */
        if (std::isdigit(c))
            return c - '0';

        return std::tolower(c) - 87;
    }
}


namespace ccomp
{
    lexer lexer::from_file(std::string_view source_path)
    {
        if (!source_path.ends_with(".c8"))
            std::cerr << "[WARNING]: Input file " << source_path << " doesn't have c8 extension.\n";

        std::ifstream input_stream(source_path.data());

        if (!input_stream)
        {
            const auto error { "Could not open file \"" + std::string{ source_path } + "\"." };
            throw std::runtime_error(error);
        }

        std::stringstream source_buffer;
        source_buffer << input_stream.rdbuf();

        return lexer(std::move(source_buffer));
    }

    lexer lexer::from_buff(std::string_view source_buffer)
    {
        std::stringstream ss(source_buffer.data());
        return lexer(std::move(ss));
    }

    lexer::lexer(std::stringstream stream)
        : m_stream(std::move(stream)),
        m_keywords {
            {"define", token_type::define },
            {"main", token_type::main_symbol },
            {"mov", token_type::instruction },
            {"add", token_type::instruction },
            {"sub", token_type::instruction },
            {"eq", token_type::instruction },
            {"neq", token_type::instruction },
            {"swp", token_type::instruction }
        }
    {}

    CCOMP_NODISCARD std::vector<token> lexer::generate_tokens()
    {
        std::vector<token> sequence;

        for (auto t { next_token() }; t.type != token_type::eof; t = next_token())
            sequence.push_back(t);

        return sequence;
    }

    CCOMP_NODISCARD token lexer::next_token()
    {
        skip_next_ws();

        const char c { peek_chr() };

        if (std::isalpha(c))
        {
            std::string lexeme { next_str_lexeme() };
            return { lexeme_to_token_type(lexeme), std::move(lexeme) };
        } else if (std::isdigit(c))
        {
            const std::size_t lexeme { next_numeric_lexeme() };
            return { token_type::number, lexeme };
        }

        next_chr();

        return { lexeme_to_token_type(c) };
    }

    CCOMP_NODISCARD std::string lexer::next_str_lexeme()
    {
        std::string lexeme;

        for (char c = next_chr(); c == '_' || std::isalnum(c); c = next_chr())
        {
            lexeme += c;

            /* next identifier's character doesn't match identifier format */
            const char next_id_char { peek_chr() };
            if (next_id_char != '_' && !std::isalnum(next_id_char))
                break;
        }

        return lexeme;
    }

    CCOMP_NODISCARD std::size_t lexer::next_numeric_lexeme()
    {
        int base { BASE_DEC };

        if (peek_chr() == '0')
        {
            /* skip 0 */
            next_chr();

            /* check given base */
            switch (peek_chr())
            {
                case 'b': base = BASE_BIN; next_chr(); break;
                case 'o': base = BASE_OCT; next_chr(); break;
                case 'x': base = BASE_HEX; next_chr(); break;
                default: m_stream.unget(); break;
            }
        }

        std::size_t num {};
        bool prev_is_quote {};

        for (char c { next_chr() }; ; c = next_chr())
        {
            if (c == '\'')
            {
                if (prev_is_quote)
                    throw lexer_error(m_state, "Multiple following quotes are forbidden");

                prev_is_quote = true;
                continue;
            }

            prev_is_quote = false;

            if (!is_valid_digit(c, base))
            {
                if (std::isalnum(c))
                    throw ccomp::numeric_base_error(m_state, c, base);

                m_stream.unget();
                break;
            }

            /* add to num */
            const uint8_t numeric_value { digit_to_value(c) };
            num = num * base + numeric_value;
        }

        return num;
    }

    void lexer::skip_next_ws()
    {
        for (char c { peek_chr() }; std::isspace(c); c = peek_chr())
            next_chr();
    }

    CCOMP_NODISCARD char lexer::peek_chr()
    {
        return static_cast<char>(m_stream.peek());
    }

    char lexer::next_chr()
    {
        const char c { static_cast<char>(m_stream.get()) };

        if (c == '\n')
        {
            ++m_state.line;
            m_state.column = 0;
        } else
            ++m_state.column;

        return c;
    }

    CCOMP_NODISCARD token_type lexer::lexeme_to_token_type(std::string_view lexeme) const
    {
        return map_value_or(m_keywords, lexeme, token_type::identifier);
    }

    CCOMP_NODISCARD token_type lexer::lexeme_to_token_type(char c) const noexcept
    {
        switch (c)
        {
            case '.': return token_type::dot;
            case ',': return token_type::comma;
            case ':': return token_type::colon;
            case '[': return token_type::angle_brackets_left;
            case ']': return token_type::angle_brackets_right;
            default: return  token_type::eof;
        }
    }

    CCOMP_NODISCARD bool lexer::is_keyword(std::string_view word) const
    {
        return m_keywords.contains(word);
    }
}
