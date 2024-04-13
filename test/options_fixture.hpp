#ifndef CHASM_OPTIONS_FIXTURE_HPP
#define CHASM_OPTIONS_FIXTURE_HPP

#include <chasm/options.hpp>


///
/// utility class to setup command line arguments
/// for tests so the tested code that uses them does not throw exceptions
///
/// if we ever want to introduce specific test environment we can create a fixture
/// like the following
///
struct options_fixture
{
	options_fixture()
	{
		const char* argv[] = { "placeholder" };

		try
		{
			//
			// We need a try-catch block here because cxxopts throws when we try to parse twice.
			// I don't want to introduce a global variable as I have no idea what cursed shit boost does behind
			// our back that may mess with it.
			//
			chasm::options::parse(1, argv);
		}
		catch (...)
		{

		}
	}

	~options_fixture() = default;
};


#endif //CHASM_OPTIONS_FIXTURE_HPP
