#ifndef CCOMP_LOG_HPP
#define CCOMP_LOG_HPP


#include <format>
#include <iostream>
#include <string_view>


namespace ccomp::log
{
    template<typename ...Args>
    void info(std::string_view fmt, Args&& ... args)
    {
        std::cout << "[INFO] " << std::vformat(fmt, std::make_format_args(args...)) << '\n';
    }

    template<typename ...Args>
    void warn(std::string_view fmt, Args&& ... args)
    {
        std::cout << "[WARN] " << std::vformat(fmt, std::make_format_args(args...)) << '\n';
    }

    template<typename ...Args>
    void error(std::string_view fmt, Args&& ... args)
    {
        std::cout << "[ERROR] " << std::vformat(fmt, std::make_format_args(args...)) << '\n';
    }
}


#endif // !CCOMP_LOG_HPP
