#ifndef CCOMP_COMMAND_LINE_HPP
#define CCOMP_COMMAND_LINE_HPP


#include <string_view>
#include <span>



namespace ccomp
{
    class command_line
    {
    public:
        static void register_args(int argc, char **argv);

        static bool has_flag(std::string_view flag_name, bool need_value = true);

        static std::string_view get_flag(std::string_view flag_name);

        static std::string get_flag_or(std::string_view flag_name, std::string default_value);

    private:
        static std::span<char *> args;
    };
}

#endif //CCOMP_COMMAND_LINE_HPP
