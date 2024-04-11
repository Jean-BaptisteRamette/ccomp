#ifndef CHASM_ASSEMBLER_ERROR_HPP
#define CHASM_ASSEMBLER_ERROR_HPP

#include <string_view>
#include <stdexcept>
#include <format>


namespace chasm
{
	struct assembler_error : std::runtime_error
	{
		explicit assembler_error(std::string_view message)
			: std::runtime_error(message.data())
		{}

		template<typename ...Args>
		explicit assembler_error(std::string_view fmt_message, Args&&... args)
			: std::runtime_error(std::vformat(fmt_message, std::make_format_args(args...)))
		{}
	};
}


#endif //CHASM_ASSEMBLER_ERROR_HPP
