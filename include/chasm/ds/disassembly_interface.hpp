#ifndef CHASM_DISASSEMBLY_INTERFACE_HPP
#define CHASM_DISASSEMBLY_INTERFACE_HPP


#include <vector>
#include <memory>

#include <chasm/ds/disassembly_graph.hpp>
#include <chasm/ds/paths.hpp>


namespace chasm::ds
{
	class disassembly_interface
	{
	public:
		explicit disassembly_interface(disassembly_graph&& graph_);

		disassembly_interface(const disassembly_interface&) = delete;
		disassembly_interface(disassembly_interface&&) = delete;
		disassembly_interface& operator=(const disassembly_interface&) = delete;
		disassembly_interface& operator=(disassembly_interface&&) = delete;

		~disassembly_interface() = default;

		void run();

	private:
		void print_disassembly() const;

	private:
		disassembly_graph graph;

		bool is_running = true;
	};
}

#endif //CHASM_DISASSEMBLY_INTERFACE_HPP
