#include <unordered_set>
#include <filesystem>
#include <fstream>

#include <ccomp/lexer.hpp>
#include <ccomp/log.hpp>


namespace ccomp
{
    namespace
    {
        std::unordered_set<std::string_view> keywords = { "define", "raw", "proc", "endp" };
        std::unordered_set<std::string_view> spec_regs = { "pc", "sp", "ar", "dt", "st" };

        std::unordered_set<std::string_view> gp_regs = {
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

        std::unordered_set<std::string_view> instructions = {
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

        token_type map_token_type(std::string_view lexeme)
        {
            if (gp_regs.contains(lexeme))
                return token_type::gp_register;

            if (instructions.contains(lexeme))
                return token_type::instruction;

			if (spec_regs.contains(lexeme))
				return token_type::special_register;

			if (keywords.contains(lexeme))
				return token_type::keyword;

			return token_type::identifier;
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

		return { .type = type, .lexeme = std::move(lexeme), .source_location = source_loc };
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
            next_chr();
            if (next_chr() != ';')
				throw std::runtime_error("Expected a second \";\" for comment.");

            skip_comment();
            c = peek_chr();
        }

        if (istream.eof())
            return make_token(token_type::eof);

        if (std::isdigit(c))
            return make_token(token_type::numerical, read_numeric_lexeme());

        if (std::isalpha(c))
        {
            const auto lexeme = read_alpha_lexeme();
            return make_token(map_token_type(lexeme), lexeme);
        }

        if (std::string_view { "[]():,." }.contains(c))
        {
            const auto tok = make_token(token_type::special_character, read_special_char());
            next_chr();
            return tok;
        }

        throw lexer_exception::undefined_token_error(make_token(token_type::undefined));
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
        while (!istream.eof() && next_chr() != '\n')
            next_chr();

        skip_wspaces();
    }

    void lexer::skip_wspaces()
    {
        for (char chr = peek_chr(); std::isspace(chr); chr = peek_chr())
            next_chr();
    }

    std::string lexer::read_numeric_lexeme()
    {
        const int base = [this]()
        {
            if (peek_chr() != '0')
                return 10;

            next_chr();

            switch (peek_chr())
            {
                case 'x': return 16;
                case 'o': return 8;
                case 'b': return 2;
                default:  return std::isdigit(peek_chr()) ? 10 : - 1;
            }
        }();

        if (base == -1)
            throw lexer_exception::numeric_base_error(cursor, peek_chr(), 10);

        auto base_has_digit = [](int base, char digit) -> bool
        {
            if (base == 10 &&  digit >= '0' && digit <= '9')  return true;
            if (base ==  8 &&  digit >= '0' && digit <= '7')  return true;
            if (base ==  2 && (digit == '0' || digit == '1')) return true;

            digit = static_cast<char>(std::tolower(digit));

            if (base == 16 && ((digit >= '0' && digit<= '9') || (digit>= 'a' && digit <= 'f')))
                return true;

            return false;
        };

        const size_t begin = istream.tellg();

        // skip base character
        if (base != 10)
            next_chr();

        for (char c = next_chr(); ; c = next_chr())
        {
            if (c == '\'')
            {
                if (std::isalnum(peek_chr()))
                    continue;

                istream.unget();
                break;
            }

            if(!std::isalnum(c))
            {
                if (c != '\0')
                    istream.unget();

                break;
            }

            if (!base_has_digit(base, c))
                throw lexer_exception::numeric_base_error(cursor, c, base);
        }

        return istream.substr(begin, istream.tellg() - begin);
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

	std::string lexer::read_special_char() const
	{
		return istream.substr(istream.tellg(), 1);
	}
}



