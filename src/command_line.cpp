#include <ccomp/command_line.hpp>
#include <iostream>


namespace ccomp
{
    std::optional<command_line_info> read_argv(int argc, char **argv)
    {
        if (argc < 2)
        {
            std::cerr << "Input file not provided\n";
            return std::nullopt;
        }

        command_line_info info;
        info.input_file_name = argv[1];

        if (argc >= 3)
            info.output_file_name = argv[2];
        else
            info.output_file_name = "a.out";

        return std::make_optional(info);
    }
}

