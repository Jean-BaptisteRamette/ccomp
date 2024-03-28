#include <vector>

#include <ccomp/command_line.hpp>
#include <ccomp/parser.hpp>
#include <ccomp/lexer.hpp>
#include <ccomp/log.hpp>



#define CMDLINE_FLAG_INPUT  "-input"
#define CMDLINE_FLAG_OUTPUT "-output"


std::string program_buffer(const std::filesystem::path& path)
{
	if (path.extension() != ".c8")
		ccomp::log::warn("Input file does not have the c8 extension");

	std::ifstream is(path);

	if (!is)
		throw std::runtime_error("Could not open source file " + path.string());

	return { std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>() };
}


int main(int argc, char** argv)
{
    ccomp::command_line::register_args(argc, argv);

    if (!ccomp::command_line::has_flag(CMDLINE_FLAG_INPUT))
    {
        ccomp::log::error("No input file");
        return EXIT_FAILURE;
    }

    const auto input_file = ccomp::command_line::get_flag(CMDLINE_FLAG_INPUT);
    const auto output_file = ccomp::command_line::get_flag_or(CMDLINE_FLAG_OUTPUT, "out.c8c");

	try
	{
		auto lexer = ccomp::lexer(program_buffer(input_file));
		auto tokens = lexer.enumerate_tokens();

		if (tokens.empty())
		{
			ccomp::log::warn("No input to be read.\n");
			return EXIT_SUCCESS;
		}

		ccomp::parser parser(std::move(tokens));

		const auto ast = parser.make_tree();
		const auto binary = ast.generate();

	} catch (std::runtime_error& error)
	{
		ccomp::log::error(error.what());
		return EXIT_FAILURE;
	}


    return EXIT_SUCCESS;
}