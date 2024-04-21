#ifndef CHASM_CHASM_EXCEPTION_HPP
#define CHASM_CHASM_EXCEPTION_HPP

#include <string_view>
#include <stdexcept>
#include <format>


namespace chasm
{
	struct chasm_exception : std::runtime_error
	{
		explicit chasm_exception(std::string_view message)
			: std::runtime_error(message.data())
		{}

		template<typename ...Args>
		explicit chasm_exception(std::string_view fmt_message, Args&&... args)
			: std::runtime_error(std::vformat(fmt_message, std::make_format_args(args...)))
		{}
	};
}


#endif //CHASM_CHASM_EXCEPTION_HPP
