#ifndef CHASM_SOURCE_LOCATION_HPP
#define CHASM_SOURCE_LOCATION_HPP

#include <string>
#include <format>


namespace chasm
{
	struct source_location
	{
		size_t col  { 1 };
		size_t line { 1 };

		void next_line()
		{
			++line;
			col = 1;
		}

		void advance()
		{
			++col;
		}

		void step_back(size_t steps = 1)
		{
			if (col >= steps)
				col -= steps;
		}
	};

	[[nodiscard]]
	inline std::string to_string(const source_location& source_loc)
	{
		return std::format("line {}, column {}", source_loc.line, source_loc.col);
	}
}


#endif //CHASM_SOURCE_LOCATION_HPP
