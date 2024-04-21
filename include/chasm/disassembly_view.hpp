#ifndef CHASM_DISASSEMBLY_VIEW_HPP
#define CHASM_DISASSEMBLY_VIEW_HPP


#include <chasm/disassembler.hpp>


namespace chasm::ds
{
	class disassembly_view
	{
	public:
		explicit disassembly_view(const disassembler&);

		disassembly_view(const disassembly_view&) = delete;
		disassembly_view(disassembly_view&&) = delete;
		disassembly_view& operator=(const disassembly_view&) = delete;
		disassembly_view& operator=(disassembly_view&&) = delete;

		~disassembly_view() = default;

		void print() const;
	};
}

#endif //CHASM_DISASSEMBLY_VIEW_HPP
