#include <chasm/ds/paths_manager.hpp>


namespace chasm::ds
{
	std::vector<path> paths_manager::paths() const
	{
		return processed;
	}

	void paths_manager::try_add_path(arch::addr path_start)
	{
		if (!was_processed(path_start))
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

	bool paths_manager::has_pending() const
	{
		return !pending.empty();
	}

	arch::addr paths_manager::next_unprocessed()
	{
		const auto r = pending.front();

		pending.pop_front();

		return r;
	}
}