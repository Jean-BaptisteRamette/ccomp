#ifndef CCOMP_LEXER_HPP
#define CCOMP_LEXER_HPP


#include <ccomp/command_line.hpp>
#include <vector>


namespace ccomp
{
    enum class token_type
    {
        eof
    };

    struct token final
    {
        token_type type;
    };


    class lexer final
    {
    public:
        explicit lexer(command_line_info info);
        ~lexer() = default;

        lexer(const lexer&) = delete;
        lexer(lexer&&) = delete;
        lexer& operator=(const lexer&) = delete;
        lexer& operator=(lexer&&) = delete;

        /*!
         * @brief analyze the file and creates a sequence of token
         * @return token sequence
         */
        std::vector<token> generate_tokens();

    private:
        command_line_info m_info;
    };
}


#endif //CCOMP_LEXER_HPP
