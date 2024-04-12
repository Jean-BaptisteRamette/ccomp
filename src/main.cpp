#include <vector>

#include <chasm/parser.hpp>
#include <chasm/lexer.hpp>
#include <chasm/arch.hpp>
#include <chasm/log.hpp>
#include <chasm/options.hpp>

std::string program_buffer(const std::filesystem::path& path)
{
	if (path.extension() != ".c8")
		chasm::log::warn("Input file does not have the c8 extension");

	std::ifstream is(path);

	if (!is)
		throw std::runtime_error("Could not open source file " + path.string() + " for reading");

	return { std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>() };
}

void write_to_file(const std::filesystem::path& file, const std::vector<chasm::arch::opcode>& binary)
{
	std::ofstream os(file, std::ios::binary);

	if (!os)
		throw std::runtime_error("Could not open file " + file.string() + " for writing");

	const auto size = binary.size() * sizeof(chasm::arch::opcode);

	if (size > chasm::arch::MAX_PROGRAM_SIZE)
		chasm::log::warn("CHIP-8 programs are generally up to {} bytes but input file assembled to {} bytes.",
						 chasm::arch::MAX_PROGRAM_SIZE,
						 size);

	os.write(reinterpret_cast<const char*>(binary.data()), static_cast<std::streamsize>(size));
}

int main(int argc, char** argv)
{
	try
	{
		chasm::options::parse(argc, argv);

    	if (!chasm::options::has_flag("in") || chasm::options::has_flag("help"))
    	{
			chasm::options::help();
    	    return EXIT_FAILURE;
    	}

		const auto ifile = chasm::options::arg<std::string>("in");
		const auto ofile = chasm::options::arg<std::string>("out");

		auto lexer  = chasm::lexer(program_buffer(ifile));
		auto tokens = lexer.enumerate_tokens();

		if (tokens.empty())
		{
			chasm::log::warn("No input to be read.\n");
			return EXIT_SUCCESS;
		}

		chasm::parser parser(std::move(tokens));

		auto ast = parser.make_tree();
		const auto binary = ast.generate();
		write_to_file(ofile, binary);

		chasm::log::info("Build of file {} to {} finished", ifile, ofile);
	}
	catch (cxxopts::exceptions::exception& error)
	{
		chasm::log::error(error.what());
		chasm::options::help();

		return EXIT_FAILURE;
	}
	catch (std::exception& error)
	{
		chasm::log::error(error.what());
		return EXIT_FAILURE;
	}


    return EXIT_SUCCESS;
}