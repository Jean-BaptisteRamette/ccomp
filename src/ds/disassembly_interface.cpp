#include <iostream>
#include <chasm/ds/disassembly_interface.hpp>


namespace chasm::ds
{
	disassembly_interface::disassembly_interface(disassembly_graph&& graph_)
		: graph(std::move(graph_))
	{}

	void disassembly_interface::run()
	{
		while (is_running)
		{
			print_disassembly();

			std::string cmd;
			std::cout << '>';
			std::cin >> cmd;

			if (cmd == "exit")
				is_running = false;
		}
	}

	void disassembly_interface::print_disassembly() const
	{
		const auto procedures = graph.get_procedures();

		for (const auto& proc : procedures)
		{
			std::cout << std::format("proc sub_{:04X}", proc.entrypoint()) << std::endl;

			for (const auto& path : proc.get_paths())
			{
				if (path.addr_start() != proc.entrypoint())
					std::cout << std::format(".loc_{:04X}:", path.addr_start()) << std::endl;

				for (size_t i = 0; i < path.instructions_count(); ++i)
					std::cout << "    " << path.symbolic(i) << std::endl;
			}

			std::cout << std::format("endp sub_{:04X}\n", proc.entrypoint()) << std::endl;
		}

		for (const auto& path : graph.get_paths())
		{
			std::cout << std::format(".loc_{:04X}:", path.addr_start()) << std::endl;

			for (size_t i = 0; i < path.instructions_count(); ++i)
				std::cout << "    " << path.symbolic(i) << std::endl;
		}
	}
}