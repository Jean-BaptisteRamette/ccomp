#include <chasm/ds/paths_manager.hpp>


namespace chasm::ds
{
	std::vector<path> paths_manager::paths() const
	{
		return processed;
	}

	void paths_manager::try_add_path(arch::addr path_start)
	{
		//
		// Only register code path that have never been analyzed yet
		//
		if (!is_pending(path_start) && !was_processed(path_start) && path_start != current)
			pending.push_back(path_start);
	}

	void paths_manager::add_processed(path&& p)
	{
		processed.push_back(std::move(p));
	}

	bool paths_manager::was_processed(arch::addr path_addr) const
	{
		return std::ranges::any_of(processed, [&](const path& other)
		{
			return other.addr_start() == path_addr;
		});
	}

	bool paths_manager::is_pending(arch::addr path_addr) const
	{
		return std::ranges::any_of(pending, [&](arch::addr other)
		{
			return other == path_addr;
		});
	}

	bool paths_manager::has_pending() const
	{
		return !pending.empty();
	}

	arch::addr paths_manager::next_pending()
	{
		const auto r = pending.front();
		current = r;

		pending.pop_front();

		return r;
	}
}