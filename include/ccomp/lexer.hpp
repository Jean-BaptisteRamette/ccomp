#ifndef CCOMP_LEXER_HPP
#define CCOMP_LEXER_HPP

#ifdef UNIT_TESTS_ON
#define CCOMP_PRIVATE public
#define CCOMP_NODISCARD
#else
#define CCOMP_PRIVATE private
#define CCOMP_NODISCARD [[nodiscard]]
#endif


#include <ccomp/command_line.hpp>
#include <unordered_map>
#include <sstream>
#include <variant>
#include <vector>


namespace ccomp
{
    enum class token_type
    {
        eof,
        main_symbol,
        define,
        label,
        identifier,
        dot,
        comma,
        colon,
        number,
        angle_brackets_left,
        angle_brackets_right,
        instruction
    };

    struct token final
    {
        token_type type;
        std::variant<std::string, std::size_t> lexeme_value;
    };

    struct lexer_state
    {
        std::size_t line { 1 };
        std::size_t column { 1 };
    };

    class lexer final
    {
    public:
        /*!
         * @brief Creates a lexer from a file path
         * @param source_path file path of the compiled file
         */
        static lexer from_file(std::string_view source_path);

        /*!
         * @brief Creates a lexer from a buffer, useful for unit tests
         * @param source_buffer content to be lexed
         */
        static lexer from_buff(std::string_view source_buffer);

        /*!
         * @brief Creates a lexer from a stream
         * @param stream contains the source content
         */
        explicit lexer(std::stringstream stream);

        ~lexer() = default;

        lexer(const lexer&) = delete;
        lexer(lexer&&) = delete;
        lexer& operator=(const lexer&) = delete;
        lexer& operator=(lexer&&) = delete;

        /*!
         * @brief performs a lexical analyze of the file and creates a sequence of tokens
         * @return token sequence
         */
        CCOMP_NODISCARD std::vector<token> generate_tokens();

    CCOMP_PRIVATE:
        /*!
         * @brief returns the peek_chr token that can be found in the stream
         */
        CCOMP_NODISCARD token next_token();

        /*!
         * @brief returns the peek_chr lexeme that can be found in the stream
         */
        CCOMP_NODISCARD std::string next_str_lexeme();

        /*!
         * @brief returns the peek_chr numeric value in the stream
         * may throw numeric_base_error
         */
        CCOMP_NODISCARD std::size_t next_numeric_lexeme();

        /*!
         * @brief skips all whitespaces
         */
        void skip_next_ws();

        /*!
         * @brief returns the next character without advancing the stream cursor
         */
        CCOMP_NODISCARD char peek_chr();

        /*!
         * @brief returns the next character in the stream and advances the stream cursor
         */
        char next_chr();

        /*!
         * @brief matches a lexeme's string representation to a token_type
         */
        CCOMP_NODISCARD token_type lexeme_to_token_type(std::string_view lexeme) const;
        CCOMP_NODISCARD token_type lexeme_to_token_type(char c) const noexcept;

        CCOMP_NODISCARD bool is_keyword(std::string_view word) const;

    CCOMP_PRIVATE:
        std::stringstream m_stream;
        lexer_state m_state;

        /* used to check if a word is reserved, and to easily map them with a token type */
        std::unordered_map<std::string_view, token_type> m_keywords;
    };
}


#endif //CCOMP_LEXER_HPP
