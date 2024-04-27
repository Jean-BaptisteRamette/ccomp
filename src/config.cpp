#include <chasm/config.hpp>
#include <chasm/log.hpp>


namespace chasm
{
	config::config()
		: default_storage {
			//
			// Default values for the config variables
			//
			{ config_vars::RAW_ALIGNED, 1 }
		},
		storage { default_storage }
	{}

	void config::reset(std::string_view id)
	{
		if (default_storage.contains(id))
			set(id, default_storage.at(id));
		else
			log::warn("Reset for config id \"{}\" ignored because id is invalid");
	}

	void config::set(std::string_view id, int v)
	{
		if (storage.contains(id))
			storage[id] = v;
		else
			log::warn("Invalid config id \"{}\", ignored", id);
	}
}

