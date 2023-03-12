#ifndef CCOMP_LEXER_HPP
#define CCOMP_LEXER_HPP

#ifdef UNIT_TESTS_ON
#define CCOMP_PRIVATE public
#define CCOMP_NODISCARD
#else
#define CCOMP_PRIVATE private
#define CCOMP_NODISCARD [[nodiscard]]
#endif


#include <sstream>
#include <memory>

#include <ccomp/stream.hpp>
#include <ccomp/error.hpp>


namespace ccomp
{
    enum class token_type
    {
        undefined,

        eof,

        // [0-9-a-f-A-F]
        number,

        // 'A' (quotes included)
        byte_ascii,

        // define, raw (instructions not included
        keyword,

        // label names and constant defined with the "define" keywords
        identifier,

        // call, ret, jmp, cls...
        instruction,

        // pc, sp, ar, dt, st
        special_register,

        // r0 - r9
        gp_register,
    };


    struct token
    {
        token_type type;
        std::string_view lexeme;
    };


    struct lexer_state
    {
        size_t col {};
        size_t row {};
    };


    class lexer final
    {
    public:
        CCOMP_NODISCARD
        static std::unique_ptr<lexer> from_file(std::string_view path, error_code& ec);

#ifdef UNIT_TESTS_ON
        CCOMP_NODISCARD
        static std::unique_ptr<lexer> from_buff(std::string_view buff);
#endif

        explicit lexer(ccomp::stream&& istream);
        ~lexer() = default;

        lexer(const lexer&)            = delete;
        lexer(lexer&&)                 = delete;
        lexer& operator=(const lexer&) = delete;
        lexer& operator=(lexer&&)      = delete;

        CCOMP_NODISCARD
        token next_token();

    CCOMP_PRIVATE:

        CCOMP_NODISCARD
        char peek_chr();  
        char next_chr();

        void skip_comment();
        void skip_wspaces();

        CCOMP_NODISCARD
        std::string_view read_numeric_lexeme();

        CCOMP_NODISCARD
        std::string_view read_alpha_lexeme();

    CCOMP_PRIVATE:

        stream istream;
        lexer_state state;
    };


};


#endif //CCOMP_LEXER_HPP
