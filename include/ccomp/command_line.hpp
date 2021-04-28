#ifndef CCOMP_COMMAND_LINE_HPP
#define CCOMP_COMMAND_LINE_HPP

#include <string_view>
#include <optional>

namespace ccomp
{
    struct command_line_info final
    {
        std::string_view input_file_name;
        std::string_view output_file_name;
    };

    /*!
     * @brief Read command line arguments
     * @return empty if error else command_line_info object
     */
    std::optional<command_line_info> read_argv(int argc, char** argv);
}

#endif //CCOMP_COMMAND_LINE_HPP
