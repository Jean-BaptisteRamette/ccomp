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
#include <variant>
#include <memory>
#include <format>

#include <ccomp/stream.hpp>
#include <ccomp/error.hpp>


namespace ccomp
{
    enum class token_type
    {
        eof,

		numerical,           // [0-9-a-f-A-F]
        byte_ascii,          // 'A' (quotes included)
        keyword_define,      // define x ...
		keyword_raw,         // raw(...)
		keyword_proc_start,  // proc ...
		keyword_proc_end,    // endp
        identifier,          // constants defined with the "define" keywords and label names
        instruction,         // call, ret, jmp, cls...
		register_name,       // special and general purpose registers
		bracket_open,
		bracket_close,
		parenthesis_open,
		parenthesis_close,
		colon,
		dot,
		comma
    };

	struct source_location
	{
		size_t col  { 1 };
		size_t line { 1 };

		void next_line()
		{
			++line;
			col = 1;
		}

		void advance()
		{
			++col;
		}

		void step_back(size_t steps = 1)
		{
			if (col >= steps)
				col -= steps;
		}
	};

	struct token
    {
        token_type type;
		source_location source_location;

		std::variant<uint16_t, std::string> data;
    };

    class lexer final
    {
    public:
        CCOMP_NODISCARD
        static std::unique_ptr<lexer> from_file(std::string_view path, error_code& ec);

#ifdef UNIT_TESTS_ON
        CCOMP_NODISCARD
        static std::unique_ptr<lexer> from_buffer(std::string_view buff);
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

		CCOMP_NODISCARD token make_token(token_type type, std::string lexeme = {}) const;
		CCOMP_NODISCARD token make_numerical_token(uint16_t numerical_value) const;

		CCOMP_NODISCARD uint16_t    read_numeric_lexeme();
        CCOMP_NODISCARD std::string read_alpha_lexeme();

    CCOMP_PRIVATE:
        ccomp::stream istream;
        source_location cursor;
    };


    namespace lexer_exception
    {
        struct invalid_digit_for_base : std::runtime_error
        {
            invalid_digit_for_base(source_location source_loc, char digit_, int base_)
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

		struct numeric_constant_too_large : std::runtime_error
		{
			numeric_constant_too_large(source_location source_loc, std::string numeric_lexeme_)
					: std::runtime_error(std::format("Numeric constant {} at line {} column {} is too large for a 16-bit value.",
													 numeric_lexeme_,
													 source_loc.line,
													 source_loc.col)),
					  numeric_lexeme(std::move(numeric_lexeme_))
			{}

			std::string numeric_lexeme;
		};

        struct undefined_character_token : std::runtime_error
        {
            explicit undefined_character_token(char c_, const source_location& location)
                : std::runtime_error(std::format("Character '{}' cannot match any token at line {} column {}.",
									 c_,
									 location.line,
									 location.col)),
				  c(c_)
            {}

            const char c;
        };
    };

	namespace
	{
		CCOMP_NODISCARD
		constexpr std::string_view to_string(token_type type)
		{
			switch (type)
			{
				case token_type::eof:
					return "eof";
				case token_type::numerical:
					return "numerical";
				case token_type::byte_ascii:
					return "ascii";
				case token_type::keyword_define:
					return "define";
				case token_type::keyword_raw:
					return "raw";
				case token_type::identifier:
					return "identifier";
				case token_type::instruction:
					return "instruction";
				case token_type::register_name:
					return "register name";
				case token_type::bracket_open:
					return "open bracket";
				case token_type::bracket_close:
					return "close bracket";
				case token_type::parenthesis_open:
					return "open parenthesis";
				case token_type::parenthesis_close:
					return "close parenthesis";
				case token_type::colon:
					return "colon";
				case token_type::comma:
					return "comma";
				case token_type::dot:
					return "dot";

				default:
					return "undefined";
			}
		}

		CCOMP_NODISCARD
		inline std::string to_string(std::initializer_list<token_type> types)
		{
			std::string joined;

			for (const auto type : types)
			{
				if (!joined.empty())
					joined += ", ";

				joined += ccomp::to_string(type);
			}

			return '(' + joined + ')';
		}

		CCOMP_NODISCARD
		inline std::string to_string(const token& token)
		{
			if (std::holds_alternative<uint16_t>(token.data))
				return std::to_string(std::get<uint16_t>(token.data));

			return std::get<std::string>(token.data);
		}
	}
};


#endif //CCOMP_LEXER_HPP
