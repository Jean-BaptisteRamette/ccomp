#include <vector>

#include <ccomp/command_line.hpp>
#include <ccomp/parser.hpp>
#include <ccomp/lexer.hpp>
#include <ccomp/error.hpp>
#include <ccomp/log.hpp>



#define CMDLINE_FLAG_INPUT  "-input"
#define CMDLINE_FLAG_OUTPUT "-output"



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

    std::vector<ccomp::token> tokens;

    for (ccomp::token token = lexer->next_token(); token.type != ccomp::token_type::eof; token = lexer->next_token())
    {
        if (token.type == ccomp::token_type::undefined)
        {
            ccomp::log::error("Undefined token {} on line {} column {}.",
                              token.lexeme,
                              lexer->state.row,
                              lexer->state.col);

            break;
        }

        tokens.push_back(token);
    }

    if (tokens.empty())
    {
        ccomp::log::warn("No input to read.\n");
        return 0;
    }

    ccomp::parser parser(std::move(tokens));

    return EXIT_SUCCESS;
}