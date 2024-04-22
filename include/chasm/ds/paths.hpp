#ifndef CHASM_PATHS_HPP
#define CHASM_PATHS_HPP

#include <string>
#include <map>

#include <chasm/arch.hpp>


namespace chasm::ds
{
	// TODO: Paths should be able to have references to each others to see control flow
	//
	//
	class path
	{
	public:
		explicit path(arch::addr start);
		~path() = default;

		void add_instruction(arch::instruction_id);
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
