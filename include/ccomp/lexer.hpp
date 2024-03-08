#ifndef CCOMP_LEXER_HPP
#define CCOMP_LEXER_HPP

#ifdef UNIT_TESTS_ON
#define CCOMP_PRIVATE public
#define CCOMP_NODISCARD
#else
#define CCOMP_PRIVATE private
#define CCOMP_NODISCARD [[nodiscard]]
#endif


#include <string_view>
#include <exception>
#include <memory>
#include <format>

#include <ccomp/stream.hpp>
#include <ccomp/error.hpp>


namespace ccomp
{
    enum class token_type
    {
        undefined,

        eof,

        // [0-9-a-f-A-F]
        numerical,

        // 'A' (quotes included)
        // TODO
        byte_ascii,

        // define, raw (instructions not included
        keyword,

        // constants defined with the "define" keywords and label names
        identifier,

        // call, ret, jmp, cls...
        instruction,

        // pc, sp, ar, dt, st
        special_register,

        // r0 - r9
        gp_register,

        // []():,.
        special_character
    };

	CCOMP_NODISCARD
	constexpr std::string_view to_string(token_type type)
	{
		switch (type)
		{
			case token_type::undefined:
				return "undefined";
			case token_type::eof:
				return "eof";
			case token_type::numerical:
				return "numerical";
			case token_type::byte_ascii:
				return "ascii";
			case token_type::keyword:
				return "keyword";
			case token_type::identifier:
				return "identifier";
			case token_type::instruction:
				return "instruction";
			case token_type::special_register:
				return "special register";
			case token_type::gp_register:
				return "general purpose register";
			case token_type::special_character:
				return "special character";

			default:
				return "unknown";
		}
	}

	struct source_location
	{
		size_t col {};
		size_t line {};
	};

	struct token
    {
        token_type type;
        std::string lexeme;
		source_location source_location;
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

		CCOMP_NODISCARD std::vector<token> enumerate_tokens();

    CCOMP_PRIVATE:
		CCOMP_NODISCARD token next_token();

        CCOMP_NODISCARD
        char peek_chr() const;
        char next_chr();

        void skip_comment();
        void skip_wspaces();

		CCOMP_NODISCARD
		token make_token(token_type type, std::string lexeme = {});

        CCOMP_NODISCARD std::string read_numeric_lexeme();
        CCOMP_NODISCARD std::string read_alpha_lexeme();
		CCOMP_NODISCARD std::string read_special_char();

    CCOMP_PRIVATE:
        ccomp::stream istream;
        source_location cursor;
    };


    namespace lexer_exception
    {
        struct numeric_base_error : std::runtime_error
        {
            numeric_base_error(source_location source_loc, char digit_, int base_)
                : std::runtime_error(std::format("Invalid digit {} for numeric base {} at line {} column {}.",
									 digit_,
									 base_,
									 source_loc.line,
									 source_loc.col)),
                  digit(digit_),
                  base(base_)
            {}

            const char digit;
            const int base;
        };

        struct undefined_token_error : std::runtime_error
        {
            explicit undefined_token_error(const token& token_)
                : std::runtime_error(std::format("Unknown token {} at line {} column {}.",
									 token_.lexeme,
									 token_.source_location.line,
									 token_.source_location.col)),
                  token(token_)
            {}

            const token token;
        };
    };
};


#endif //CCOMP_LEXER_HPP
