#include <ccomp/instructions.hpp>


namespace ccomp::inst
{
	unsigned char get_operands_count(std::string_view mnemonic)
	{
		return operands_count.at(mnemonic);
	}

	unsigned char regname2index(std::string_view regname)
	{
		return 0;
	}
}