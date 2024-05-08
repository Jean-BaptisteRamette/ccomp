#include <vector>

#include <chasm/ds/disassembly_interface.hpp>
#include <chasm/ds/disassembler.hpp>
#include <chasm/options.hpp>
#include <chasm/parser.hpp>
#include <chasm/lexer.hpp>
#include <chasm/arch.hpp>
#include <chasm/log.hpp>

namespace io
{
	std::string content(const std::filesystem::path& path)
	{
		if (path.extension() != ".c8")
			chasm::log::warn("Input file does not have the c8 extension");

		std::ifstream is(path);

		if (!is)
			throw std::runtime_error("Could not open source file " + path.string() + " for reading");

		return { std::istreambuf_iterator<char>(is), std::istreambuf_iterator<char>() };
	}

	std::vector<uint8_t> bytes(const std::filesystem::path& path)
	{
		std::basic_ifstream<uint8_t> is(path, std::ios::binary);

		if (!is)
			throw std::runtime_error("Could not open assembled source file " + path.string() + " for reading");

		return { std::istreambuf_iterator<uint8_t>(is), std::istreambuf_iterator<uint8_t>() };
	}

	void write(const std::filesystem::path& file, const std::vector<uint8_t>& binary)
	{
		std::ofstream os(file, std::ios::binary);

		if (!os)
			throw std::runtime_error("Could not open file " + file.string() + " for writing");

		if (binary.size() > chasm::arch::MAX_PROGRAM_SIZE)
			chasm::log::warn("CHIP-8 programs are generally up to {} bytes but input file assembled to {} bytes.",
							 chasm::arch::MAX_PROGRAM_SIZE,
							 binary.size());

		os.write(reinterpret_cast<const char*>(binary.data()), static_cast<std::streamsize>(binary.size()));
	}

	void hexdump(const std::vector<uint8_t>& binary)
	{
		const auto offset  = chasm::options::arg<chasm::arch::addr>("relocate");
		const auto perline = chasm::options::arg<unsigned int>("hex");
		const auto binsize = static_cast<chasm::arch::size_type>(binary.size());

		for (chasm::arch::addr curr = 0; curr < binsize; curr += perline)
		{
			std::cout << std::format("0x{:04X}: ", offset + curr);

			for (int j = 0; j < perline; ++j)
				if (curr + j < binsize)
					std::cout << std::format("{:02X} ", binary[curr + j]);

			std::cout << '\n';
		}
	}
}

int main(int argc, char** argv)
{
	try
	{
		chasm::options::parse(argc, argv);

		if (chasm::options::has_flag("help"))
			chasm::options::help();

		if (chasm::options::has_flag("in"))
		{
			const auto ifile = chasm::options::arg<std::string>("in");
			const auto ofile = chasm::options::arg<std::string>("out");

			auto lexer  = chasm::lexer(io::content(ifile));
			auto tokens = lexer.enumerate_tokens();

			if (tokens.empty())
			{
				chasm::log::warn("No input to be read.\n");
				return EXIT_SUCCESS;
			}

			auto parser = chasm::parser(std::move(tokens));
			auto ast = parser.make_tree();
			const auto binary = ast.generate();

			if (chasm::options::has_flag("hex"))
				io::hexdump(binary);

			io::write(ofile, binary);

			chasm::log::info("Build of file {} to {} finished", ifile, ofile);
		}
		else if (chasm::options::has_flag("dis"))
    	{
			auto bytes = io::bytes(chasm::options::arg<std::string>("dis"));

			if (bytes.empty())
			{
				chasm::log::warn("No input to be read.\n");
				return EXIT_SUCCESS;
			}

			auto disassembler = chasm::ds::disassembler(std::move(bytes), chasm::options::arg<chasm::arch::addr>("relocate"));
			auto interface = chasm::ds::disassembly_interface(disassembler.get_graph());
			interface.run();
    	}
		else
		{
			chasm::options::help();
			return EXIT_FAILURE;
		}
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