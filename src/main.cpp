#include <iostream>
#include <ccomp/command_line.hpp>
#include <ccomp/lexer.hpp>
#include <ccomp/error.hpp>
#include <ccomp/log.hpp>



#define CMDLINE_FLAG_INPUT  "-input"
#define CMDLINE_FLAG_OUTPUT "-output"



int main(int argc, char** argv)
{
    ccomp::cmd_line::parser cmdline_reader(argc, argv);

    if (!cmdline_reader.has_flag(CMDLINE_FLAG_INPUT))
    {
        ccomp::log::error("No input file");
        return EXIT_FAILURE;
    }

    const auto input_file = cmdline_reader.get_flag(CMDLINE_FLAG_INPUT);
    const auto output_file = cmdline_reader.get_flag_or(CMDLINE_FLAG_OUTPUT, "out.c8c");

    auto ec = ccomp::error_code::ok;
    auto lexer = ccomp::lexer::from_file(input_file, ec);

    if (!lexer)
    {
        if (ec == ccomp::error_code::file_not_found_err)
            ccomp::log::error("File {} not found.", input_file);
        else if (ec == ccomp::error_code::io_err)
            ccomp::log::error("Could not read file {}.", input_file);

        return EXIT_FAILURE;
    }

    const auto token = lexer->next_token();

    return EXIT_SUCCESS;
}