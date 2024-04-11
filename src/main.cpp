#include <vector>

#include <chasm/command_line.hpp>
#include <chasm/parser.hpp>
#include <chasm/lexer.hpp>
#include <chasm/log.hpp>


std::string program_buffer(const std::filesystem::path& path)
{
	if (path.extension() != ".c8")
		chasm::log::warn("Input file does not have the c8 extension");

	std::ifstream is(path);

	if (!is)
		throw std::runtime_error("Could not open source file " + path.string());

	return { std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>() };
}


int main(int argc, char** argv)
{
    chasm::command_line::register_args(argc, argv);

    if (!chasm::command_line::has_flag("-in"))
    {
        chasm::log::error("No input file");
        return EXIT_FAILURE;
    }

    const auto input_file = chasm::command_line::get_flag("-in");
    const auto output_file = chasm::command_line::get_flag_or("-out", "out.c8c");

	try
	{
		auto lexer = chasm::lexer(program_buffer(input_file));
		auto tokens = lexer.enumerate_tokens();

		if (tokens.empty())
		{
			chasm::log::warn("No input to be read.\n");
			return EXIT_SUCCESS;
		}

		chasm::parser parser(std::move(tokens));

		auto ast = parser.make_tree();
		const auto binary = ast.generate();

	} catch (std::runtime_error& error)
	{
		chasm::log::error(error.what());
		return EXIT_FAILURE;
	}


    return EXIT_SUCCESS;
}