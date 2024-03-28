#ifndef CCOMP_INSTRUCTIONS_HPP
#define CCOMP_INSTRUCTIONS_HPP


#include <unordered_map>
#include <string_view>


namespace ccomp::inst
{
	namespace
	{
		const std::unordered_map<std::string_view, unsigned short> operands_count = {
				{ "add",   2 },
				{ "sub",   2 },
				{ "suba",  2 },
				{ "dec",   1 },
				{ "inc",   1 },
				{ "or",    2 },
				{ "and",   2 },
				{ "xor",   2 },
				{ "shr",   1 },
				{ "shl",   1 },
				{ "rdump", 1 },
				{ "rload", 1 },
				{ "mov",   2 },
				{ "swp",   2 },
				{ "draw",  3 },
				{ "cls",   0 },
				{ "rand",  2 },
				{ "bcd",   1 },
				{ "wkey",  1 },
				{ "ske",   1 },
				{ "snke",  1 },
				{ "ret",   0 },
				{ "jmp",   1 },
				{ "call",  1 },
				{ "se",    2 },
				{ "sne",   2 }
		};
	}

    unsigned char get_operands_count(std::string_view mnemonic);
	unsigned char regname2index(std::string_view regname);
}

#endif //CCOMP_INSTRUCTIONS_HPP
