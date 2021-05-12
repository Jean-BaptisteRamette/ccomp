#ifndef CCOMP_ERROR_HPP
#define CCOMP_ERROR_HPP

#include <stdexcept>


namespace ccomp
{
    struct lexer_error : public std::runtime_error
    {
        lexer_error(lexer_state state_, const char* msg)
            : std::runtime_error(msg), state(state_)
        {}

        lexer_state state;
    };

    struct numeric_base_error : lexer_error
    {
        numeric_base_error(lexer_state state_, char digit_, int base_)
            : lexer_error(state_, "Invalid digit for numeric base"),
            digit(digit_),
            base(base_)
        {}

        char digit;
        int base;
    };

    struct invalid_token_error : lexer_error
    {
        invalid_token_error(lexer_state state_, std::string lexeme_)
            : lexer_error(state_, "Invalid token"),
            lexeme(std::move(lexeme_))
        {}

        std::string lexeme;
    };
}

#endif //CCOMP_ERROR_HPP
