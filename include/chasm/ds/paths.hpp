#ifndef CHASM_PATHS_HPP
#define CHASM_PATHS_HPP

#include <string>
#include <map>

#include <chasm/arch.hpp>
#include <chasm/ds/formatter.hpp>


namespace chasm::ds
{
	class path
	{
	public:
		explicit path(arch::addr start);
		~path() = default;


		template<std::integral ...Args>
		void add_instruction(arch::instruction_id id, arch::operands_mask mask, Args...args)
		{
			disassembly.emplace_back(
						ds::formatter::format(id, mask, args...)
					);
		}

		void mark_end();

		[[nodiscard]] arch::addr addr_start() const;
		[[nodiscard]] arch::addr addr_end() const;
		[[nodiscard]] bool ended() const;
		[[nodiscard]] size_t instructions_count() const;
		[[nodiscard]] std::string symbolic(size_t instruction_index) const;

	private:
		arch::addr start_addr;
		std::vector<std::string> disassembly;
		bool ended_ {};
	};
}

#endif //CHASM_PATHS_HPP
