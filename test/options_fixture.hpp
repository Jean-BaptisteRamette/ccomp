#ifndef CHASM_OPTIONS_FIXTURE_HPP
#define CHASM_OPTIONS_FIXTURE_HPP

#include <chasm/options.hpp>


/// utility class to setup command line arguments
/// for tests so the tested code that uses them does not throw exceptions

//
// We need a try-catch block here because cxxopts throws when we try to parse twice.
// I don't want to introduce a global variable as I have no idea what cursed shit boost does behind
// our back that may mess with it.
//
#define MAKE_OPTIONS_FIXTURE(fixture_name, ...)       \
struct fixture_name                                   \
{                                                     \
	fixture_name()                                    \
	{                                                 \
		const char* argv[] = {                        \
			"placeholder.exe",                        \
		    __VA_ARGS__                               \
		};                                            \
		                                              \
		try                                           \
		{                                             \
			chasm::options::parse(                    \
				sizeof(argv) / sizeof(char*),         \
				argv                                  \
			);                                        \
		}                                             \
		catch (...)                                   \
		{											  \
													  \
		}											  \
	}												  \
													  \
	~fixture_name() = default;						  \
}

namespace test_env
{
	MAKE_OPTIONS_FIXTURE(default_options, "placeholder");
	MAKE_OPTIONS_FIXTURE(zero_relocate, "--relocate=0x0000");
	MAKE_OPTIONS_FIXTURE(padded_sprites, "--pad-sprites");
}


#endif //CHASM_OPTIONS_FIXTURE_HPP
