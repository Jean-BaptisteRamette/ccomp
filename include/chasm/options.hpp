#ifndef CHASM_OPTIONS_HPP
#define CHASM_OPTIONS_HPP


#include <iostream>
#include <string>

#include <chasm/cxxopts.hpp>


namespace chasm
{
	class options
	{
	public:
		static void parse(int argc, const char* const* argv)
		{
			opts.add_options()
					("h,help", "Show help message")
					("in", "chasm source file to assemble", cxxopts::value<std::string>())
					("out", "The generated machine code output file path", cxxopts::value<std::string>()->default_value("out.c8c"))
					("pad-sprites", "Pad odd sized sprites")
					("hex", "Hexdumps the generated machine code, argument is the amount of opcodes per line", cxxopts::value<unsigned int>()->implicit_value("4"))
					("symbols", "Generate a file with symbols location in memory/machine code", cxxopts::value<std::string>()->implicit_value("out.c8s"))
					("relocate", "Address in which the binary is supposed to be loaded", cxxopts::value<chasm::arch::addr>()->default_value("0x200"))
					("super", "Specify the target ISA to be the SUPER-CHIP and removes warning when using non CHIP-8 instructions");

			parameters = opts.parse(argc, argv);
		}

		static void help()
		{
			std::cout << opts.help() << std::endl;
		}

		static inline bool has_flag(const std::string& flag)
		{
			return parameters.count(flag) > 0;
		}

		template<typename T>
		static inline const T& arg(const std::string& name)
		{
			return parameters[name].as<T>();
		}

	private:
		inline static cxxopts::Options opts { "chasm", "Assembler for the CHIP-8 virtual machine" };
		inline static cxxopts::ParseResult parameters;
	};
}


#endif //CHASM_OPTIONS_HPP
