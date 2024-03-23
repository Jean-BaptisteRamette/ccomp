#include <unordered_set>
#include <unordered_map>
#include <filesystem>
#include <fstream>

#include <ccomp/lexer.hpp>
#include <ccomp/log.hpp>


namespace ccomp
{
    namespace
    {
		using lexeme_set = std::unordered_set<std::string_view>;
		using lexeme_map = std::unordered_map<std::string_view, token_type>;

        const lexeme_map keywords = {
				{ "define", token_type::keyword_define     },
				{ "raw",    token_type::keyword_raw        },
				{ "proc",   token_type::keyword_proc_start },
				{ "endp",   token_type::keyword_proc_end   }
		};

        const lexeme_set special_regs_name = {
			"pc",
			"sp",
			"ar",
			"dt",
			"st"
		};

        const lexeme_set general_regs_name = {
            "r0",
            "r1",
            "r2",
            "r3",
            "r4",
            "r5",
            "r6",
            "r7",
            "r8",
            "r9",
            "ra",
            "rb",
            "rc",
            "rd",
            "re",
            "rf"
        };

        const lexeme_set instructions = {
            "add",
            "sub",
            "suba",
            "or",
            "and",
            "xor",
            "shr",
            "shl",
            "rdump",
            "rload",
            "mov",
            "swp",
            "draw",
            "cls",
            "rand",
            "bcd",
            "wkey",
            "ske",
            "skne",
            "ret",
            "jmp",
            "call",
			"se",
			"sne"
        };

		const std::unordered_map<char, token_type> special_characters = {
				{ '[', token_type::bracket_open },
				{ ']', token_type::bracket_close },
				{ '(', token_type::parenthesis_open },
				{ ')', token_type::parenthesis_close},
				{ '.', token_type::dot},
				{ ':', token_type::colon},
				{ ',', token_type::comma}
		};

        token_type map_token_type(std::string_view lexeme)
        {
            if (general_regs_name.contains(lexeme) || special_regs_name.contains(lexeme))
                return token_type::register_name;

            if (instructions.contains(lexeme))
                return token_type::instruction;

			if (keywords.contains(lexeme))
				return keywords.at(lexeme);

			return token_type::identifier;
        }

		bool base_has_digit(int base, char digit)
		{
			if (base == 10 &&  digit >= '0' && digit <= '9')  return true;
			if (base ==  8 &&  digit >= '0' && digit <= '7')  return true;
			if (base ==  2 && (digit == '0' || digit == '1')) return true;

			digit = static_cast<char>(std::tolower(digit));

			if (base == 16 && ((digit >= '0' && digit<= '9') || (digit>= 'a' && digit <= 'f')))
				return true;

			return false;
		}
    }


    std::unique_ptr<lexer> lexer::from_file(std::string_view path, error_code& ec)
    {
        if (!path.ends_with(".c8"))
            log::warn("Input file {} should have .c8 extension", path);

        if (!std::filesystem::exists(path))
        {
            ec = error_code::file_not_found_err;
            return nullptr;
        }

        std::ifstream infile(path.data());

        if (!infile)
        {
            ec = error_code::io_err;
            return nullptr;
        }

        ccomp::stream istream(infile);

		ec = error_code::ok;

        return std::make_unique<lexer>(std::move(istream));
    }


#ifdef UNIT_TESTS_ON
    std::unique_ptr<lexer> lexer::from_buffer(std::string_view buff)
    {
        ccomp::stream istream(buff.data());
        return std::make_unique<lexer>(std::move(istream));
    }
#endif

    lexer::lexer(ccomp::stream&& istream) : istream(std::move(istream)) {}

	token lexer::make_token(token_type type, std::string lexeme) const
	{
		source_location source_loc = cursor;
		source_loc.step_back(lexeme.size());

		return { .type = type, .source_location = source_loc, .data = std::move(lexeme) };
	}

	token lexer::make_numerical_token(uint16_t numerical_value) const
	{
		return { .type = token_type::numerical, .source_location = cursor, .data = numerical_value };
	}

	std::vector<token> lexer::enumerate_tokens()
	{
		std::vector<token> tokens;

		for (auto token = next_token(); token.type != token_type::eof; token = next_token())
			tokens.push_back(token);

		return tokens;
	}

    token lexer::next_token()
    {
        skip_wspaces();

        char c = peek_chr();

        if (c == ';')
        {
			do
			{
				next_chr();
				if (next_chr() != ';')
					throw std::runtime_error("Expected a second \";\" for comment.");

				skip_comment();
				c = peek_chr();
			} while (c == ';');
        }

        if (istream.eof())
            return make_token(token_type::eof);

        else if (std::isdigit(c))
            return make_numerical_token(read_numeric_lexeme());

        else if (std::isalpha(c))
        {
            const auto lexeme = read_alpha_lexeme();
            return make_token(map_token_type(lexeme), lexeme);
        }
        else if (special_characters.contains(c))
        {
			next_chr();
            return make_token(special_characters.at(c));
        }

        throw lexer_exception::undefined_character_token(c, cursor);
    }

    char lexer::peek_chr() const
    {
        return istream.peek();
    }

    char lexer::next_chr()
    {
        const char chr = istream.get();

        if (chr == '\n')
			cursor.next_line();
        else
			cursor.advance();

        return chr;
    }

    void lexer::skip_comment()
    {
        // Assumes comment was already detected
        while (!istream.eof() && peek_chr() != '\n')
            next_chr();

        skip_wspaces();
    }

    void lexer::skip_wspaces()
    {
        for (char chr = peek_chr(); std::isspace(chr); chr = peek_chr())
            next_chr();
    }

    uint16_t lexer::read_numeric_lexeme()
    {
        const auto base = [this]() -> int
        {
            if (peek_chr() != '0')
                return 10;

            next_chr();

            switch (peek_chr())
            {
                case 'x': return 16;
                case 'o': return 8;
                case 'b': return 2;

                default:
					break;
            }

			if (std::isdigit(peek_chr()))
				return 10;

			throw lexer_exception::invalid_digit_for_base(peek_chr(), 10, cursor);
        }();

		std::string numeric_lexeme;

		if (base != 10)
			next_chr();

        for (char c = next_chr(); ; c = next_chr())
        {
			if (std::isalnum(c))
			{
				if (!base_has_digit(base, c))
					throw lexer_exception::invalid_digit_for_base(c, base, cursor);

				numeric_lexeme += c;
			}
			else if (c == '\'')
			{
				if (!std::isalnum(peek_chr()))
				{
					istream.unget();
					break;
				}
			}
			else
           	{
           		if (c != '\0')
           	        istream.unget();

				break;
           	}
        }

		uint16_t constant_value {};
		const auto [_, ec] = std::from_chars(
									numeric_lexeme.data(),
									numeric_lexeme.data() + numeric_lexeme.size(),
									constant_value,
									base
							 );

		if (ec == std::errc::result_out_of_range)
			throw lexer_exception::numeric_constant_too_large(numeric_lexeme, cursor);

		return constant_value;
    }

    std::string lexer::read_alpha_lexeme()
    {
        const size_t begin = istream.tellg();

        for (char c = next_chr(); c == '_' || std::isalnum(c); c = next_chr());

        if (!istream.eof())
		{
			cursor.step_back();
			istream.unget();
		}

        return istream.substr(begin, istream.tellg() - begin);
    }
}
