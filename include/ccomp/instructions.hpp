#ifndef CCOMP_INSTRUCTIONS_HPP
#define CCOMP_INSTRUCTIONS_HPP


#include <unordered_map>
#include <string_view>


namespace ccomp::inst
{

    char operands_count(std::string_view mnemonic)
    {
        static std::unordered_map<std::string_view, char> op_count =
        {
            {"add", 2},
            {"sub", 2}
        };

        const auto it = op_count.find(mnemonic);

        if (it == std::end(op_count))
            return -1;

        return it->second;
    }
}

#endif //CCOMP_INSTRUCTIONS_HPP
