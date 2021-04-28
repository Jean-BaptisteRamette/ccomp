#include <iostream>
#include <ccomp/command_line.hpp>
#include <ccomp/lexer.hpp>
#include <ccomp/error.hpp>


int main(int argc, char** argv)
{
    const std::optional result {ccomp::read_argv(argc, argv) };

    if (!result)
        return EXIT_FAILURE;

    const auto& info { result.value() };

    try
    {
        const ccomp::lexer lexer(info);
        // const ccomp::parser parser(lexer);

    } catch (const ccomp::error& compile_error)
    {
        
    }


    return EXIT_SUCCESS;
}