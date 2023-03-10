#include <filesystem>
#include <fstream>

#include <ccomp/lexer.hpp>
#include <ccomp/log.hpp>


namespace ccomp
{
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

		std::stringstream istream;
		istream << infile.rdbuf();

		return std::make_unique<lexer>(std::move(istream));
	}


#ifdef UNIT_TESTS_ON
	std::unique_ptr<lexer> lexer::from_buff(std::string_view buff)
	{
		std::stringstream istream(buff.data());
		return std::make_unique<lexer>(std::move(istream));
	}
#endif

	lexer::lexer(std::stringstream&& istream) : istream(std::move(istream)) {}

    token lexer::next_token()
    {
        skip_wspaces();

        if (next_chr() == ';' && peek_chr() == ';')
            skip_comment();

        const auto c = next_chr();

        if (istream.eof())
            return { token_type::eof };

        return { token_type::eof };
    }

	char lexer::peek_chr()
	{
		return static_cast<char>(istream.peek());
	}

	char lexer::next_chr()
	{
		const auto chr = static_cast<char>(istream.get());

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
	}

	void lexer::skip_wspaces()
	{
		for (char chr = peek_chr(); std::isspace(chr); chr = peek_chr())
			next_chr();
	}
}



