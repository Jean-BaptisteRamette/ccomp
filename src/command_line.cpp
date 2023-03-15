#include <ccomp/command_line.hpp>


namespace ccomp
{
    std::span<char*> command_line::args;

    void command_line::register_args(int argc, char** argv)
    {
        args = { argv, static_cast<size_t>(argc) };
    }

    bool command_line::has_flag(std::string_view flag_name, bool need_value)
    {
        const auto it = std::find(args.begin(), args.end(), flag_name);

        if (it == args.end())
            return false;

        if (need_value && it + 1 == args.end())
            return false;

        return true;
    }

    std::string_view command_line::get_flag(std::string_view flag_name)
    {
        auto it = std::find(args.begin(), args.end(), flag_name);

        if (it != args.end() && it + 1 != args.end())
            return *(it + 1);

        return {};
    }

    std::string_view command_line::get_flag_or(std::string_view flag_name, std::string_view default_value)
    {
        if (!has_flag(flag_name))
            return default_value;

        const auto value = get_flag(flag_name);

        if (!value.empty())
            return value;

        return default_value;
    }

}

