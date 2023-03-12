#ifndef CCOMP_COMMAND_LINE_HPP
#define CCOMP_COMMAND_LINE_HPP


#include <string_view>
#include <span>



namespace ccomp::cmd_line
{
    class parser
    {
    public:
        parser(int argc, char** argv);

        bool has_flag(std::string_view flag_name, bool need_value = true);
        std::string_view get_flag(std::string_view flag_name);
        std::string_view get_flag_or(std::string_view flag_name, std::string_view default_value);

    private:
        std::span<char*> args;
    };
}

#endif //CCOMP_COMMAND_LINE_HPP
