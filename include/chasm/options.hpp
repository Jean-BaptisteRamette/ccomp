#ifndef CHASM_OPTIONS_HPP
#define CHASM_OPTIONS_HPP

#include <string_view>
#include <span>
#include <utility>


namespace chasm::options
{
	std::span<char*> args;

	void parse(int argc, char** argv)
	{
		args = { argv, argv + argc };
	}

	[[nodiscard]]
	bool has_flag(std::string_view flag)
	{
		const auto prefixed = "-" + std::string(flag);

		return std::ranges::find(args, prefixed.data()) != args.end();
	}

	[[nodiscard]]
	bool has_option(std::string_view option)
	{
		const auto prefixed = "-" + std::string(option);
		auto it = std::ranges::find(args, prefixed.data());

		return it != args.end() && ++it != args.end();
	}

	[[nodiscard]]
	std::string option(std::string_view opt)
	{
		const auto prefixed = "-" + std::string(opt);
		auto it = std::ranges::find(args, prefixed.data());

		return *(++it);
	}

	[[nodiscard]]
	std::string option_or(std::string_view opt, std::string def)
	{
		return has_option(opt) ? option(opt) : std::move(def);
	}
}

#endif //CHASM_OPTIONS_HPP
