#include <iostream>
#include <ccomp/command_line.hpp>
#include <ccomp/lexer.hpp>
#include <ccomp/error.hpp>


int main(int argc, char** argv)
{
    const std::optional result { ccomp::read_argv(argc, argv) };

    if (!result)
        return EXIT_FAILURE;

    const auto& info { result.value() };

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


    return EXIT_SUCCESS;
}