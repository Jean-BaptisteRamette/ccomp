#include <unordered_set>
#include <filesystem>
#include <fstream>

#include <ccomp/lexer.hpp>
#include <ccomp/log.hpp>


namespace ccomp
{
	namespace
	{
		std::unordered_set<std::string_view> keywords = { "define", "main", "raw" };
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
			"kpw",
			"eq",
			"neq",
			"ret",
			"jmp",
			"call"
		};

		token_type map_token_type(std::string_view lexeme)
		{
			if (keywords.contains(lexeme))
				return token_type::keyword;

			if (spec_regs.contains(lexeme))
				return token_type::special_register;

			if (gp_regs.contains(lexeme))
				return token_type::gp_register;

			if (instructions.contains(lexeme))
				return token_type::instruction;

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

		return std::make_unique<lexer>(std::move(istream));
	}


#ifdef UNIT_TESTS_ON
	std::unique_ptr<lexer> lexer::from_buff(std::string_view buff)
	{
		ccomp::stream istream(buff.data());
		return std::make_unique<lexer>(std::move(istream));
	}
#endif

	lexer::lexer(ccomp::stream&& istream) : istream(std::move(istream)) {}

    token lexer::next_token()
    {
		skip_wspaces();

		char c = peek_chr();

		if (c == ';')
		{
			next_chr();
			if (next_chr() != ';')
				return { token_type::undefined };

			skip_comment();
			c = peek_chr();
		}

		if (istream.eof())
			return { token_type::eof };

		if (std::isdigit(c))
			return { token_type::number, read_numeric_lexeme() };

		if (std::isalpha(c))
		{
			const auto lexeme = read_alpha_lexeme();
			return { map_token_type(lexeme), lexeme };
		}

        return { token_type::undefined };
    }

	char lexer::peek_chr()
	{
		return istream.peek();
	}

	char lexer::next_chr()
	{
		const char chr = istream.get();

        if (chr != '\n')
            ++state.col;
        else
        {
            ++state.row;
            state.col = 0;
        }

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

	std::string_view lexer::read_numeric_lexeme()
	{
		return "0";
	}

	std::string_view lexer::read_alpha_lexeme()
	{
		const auto beg = istream.iterator();
		size_t size = 0;

		for (char c = next_chr(); c == '_' || std::isalnum(c); c = next_chr())
			++size;

		return { beg, size };
	}
}



