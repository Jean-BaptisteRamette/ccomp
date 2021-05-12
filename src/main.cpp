#include <iostream>
#include <ccomp/command_line.hpp>
#include <ccomp/lexer.hpp>
#include <ccomp/error.hpp>



#define CMDLINE_FLAG_INPUT  "-input"
#define CMDLINE_FLAG_OUTPUT "-output"



int main(int argc, char** argv)
{
    ccomp::cmd_line::parser cmdline_reader(argc, argv);

    if (!cmdline_reader.has_flag(CMDLINE_FLAG_INPUT))
    {
        std::cerr << "No input file";
        return EXIT_FAILURE;
    }


    const auto input_file = cmdline_reader.get_flag(CMDLINE_FLAG_INPUT);
    const auto output_file = cmdline_reader.get_flag_or(CMDLINE_FLAG_OUTPUT, "out.c8c");

    /*
    try
    {
        auto lexer = ccomp::lexer::from_file(info.input_file_name);

        const auto tokens_seq { lexer.generate_tokens() };

        for (const auto& token : tokens_seq)
        {
            if (token.type == ccomp::token_type::number)
                std::cout << "Token Number: " << std::get<1>(token.lexeme_value) << std::endl;
            else
                std::cout << "Token Value: " << std::get<0>(token.lexeme_value) << std::endl;
        }

    } catch (const std::runtime_error& compile_error)
    {
        std::cout << compile_error.what() << std::endl;
        return EXIT_FAILURE;
    }
    */


    return EXIT_SUCCESS;
}