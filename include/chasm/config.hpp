#ifndef CHASM_CONFIG_HPP
#define CHASM_CONFIG_HPP


#include <unordered_map>
#include <string_view>
#include <concepts>

#include <chasm/chasm_exception.hpp>


namespace chasm
{
	namespace config_vars
	{
		constexpr std::string_view RAW_ALIGNED = "RAW_ALIGNED";
	}

	class config
	{
	public:
		config();
		~config() = default;

		void reset(std::string_view id);
		void set(std::string_view id, int);

		template<std::integral T>
		[[nodiscard]] T get_as(std::string_view id) const
		{
			if (!storage.contains(id))
				throw chasm_exception("Invalid config id \"{}\", unexpected", id);

			return static_cast<T>(storage.at(id));
		}

	private:
		const std::unordered_map<std::string_view, int> default_storage;
		std::unordered_map<std::string_view, int> storage;
	};
}


#endif //CHASM_CONFIG_HPP
