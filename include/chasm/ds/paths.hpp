#ifndef CHASM_PATHS_HPP
#define CHASM_PATHS_HPP

#include <string>
#include <vector>
#include <set>

#include <chasm/arch.hpp>
#include <chasm/ds/formatter.hpp>


namespace chasm::ds
{
	class path
	{
	public:
		explicit path(arch::addr);
		virtual ~path() = default;

		path(path&&) = default;
		path(const path&) = default;
		path& operator=(path&&) = default;
		path& operator=(const path&) = default;

		bool operator<(const path& other) const;

		template<std::integral ...Args>
		void add_instruction(arch::instruction_id id, arch::operands_mask mask, Args...args)
		{
			disassembly.emplace_back(
						ds::formatter::format(id, mask, args...)
					);
		}

		[[nodiscard]] arch::addr addr_start() const;
		[[nodiscard]] arch::addr addr_end() const;
		[[nodiscard]] size_t instructions_count() const;
		[[nodiscard]] std::string symbolic(size_t instruction_index) const;

	private:
		arch::addr start_addr;
		std::vector<std::string> disassembly;
	};

	class procedure
	{
	public:
		explicit procedure(arch::addr ep);
		virtual ~procedure() = default;

		bool operator<(const procedure& other) const;

		[[nodiscard]] arch::addr entrypoint() const;
		[[nodiscard]] std::set<path> get_paths() const;
		void insert_path(path p);

	private:
		arch::addr ep;
		std::set<path> ordered_paths;
	};
}

#endif //CHASM_PATHS_HPP
