#ifndef CHASM_PATHS_MANAGER_HPP
#define CHASM_PATHS_MANAGER_HPP

#include <deque>
#include <chasm/ds/paths.hpp>


namespace chasm::ds
{
	class paths_manager
	{
	public:
		paths_manager() = default;
		~paths_manager() = default;

		[[nodiscard]] std::vector<path> paths() const;

		void add_processed(path&& p);

		/// Adds code path if not processed yet
		///
		void try_add_path(arch::addr path_addr);

		/// Checks for un-analyzed code paths
		///
		/// \return
		[[nodiscard]] bool has_pending() const;

		[[nodiscard]] arch::addr next_pending();

	private:
		/// Checks if the given path has already been analyzed:
		///
		/// \return
		[[nodiscard]] bool was_processed(arch::addr path_addr) const;
		[[nodiscard]] bool is_pending(arch::addr path_addr) const;

	private:
		std::vector<path> processed;
		std::deque<arch::addr> pending;
		arch::addr current = UINT16_MAX;
	};
}


#endif //CHASM_PATHS_MANAGER_HPP
