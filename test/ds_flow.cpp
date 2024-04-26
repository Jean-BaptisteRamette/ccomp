#include <boost/test/unit_test.hpp>
#include <chasm/lexer.hpp>
#include <chasm/parser.hpp>
#include <chasm/ds/disassembler.hpp>

#include "options_fixture.hpp"


namespace details
{
	using namespace chasm;

	std::vector<uint8_t>
	codegen(std::string&& source)
	{
		auto lex = lexer(std::move(source));
		auto par = parser(lex.enumerate_tokens());
		auto ast = par.make_tree();

		return ast.generate();
	}

	std::vector<ds::path>
	make_paths(std::string&& source)
	{
		auto ds = ds::disassembler(codegen(std::move(source)));
		return ds.code_paths();
	}
}


BOOST_FIXTURE_TEST_SUITE(paths_checker, test_env::zero_relocate)

	BOOST_AUTO_TEST_CASE(test_basic_chained_jumps)
	{
		const auto paths = details::make_paths(
				".main:        \n"
				"	jmp @L1    \n"
				".L1:          \n"
				"	jmp @L2    \n"
				".L2:          \n"
				"	jmp @L3    \n"
				".L3:          \n"
				"	jmp @L4    \n"
				".L4:          \n"
				"	exit       \n");

		BOOST_CHECK_EQUAL(paths.size(), 5);
		BOOST_CHECK_EQUAL(paths[0].addr_start(), 0);
		BOOST_CHECK_EQUAL(paths[1].addr_start(), 2);
		BOOST_CHECK_EQUAL(paths[2].addr_start(), 4);
		BOOST_CHECK_EQUAL(paths[3].addr_start(), 6);
		BOOST_CHECK_EQUAL(paths[4].addr_start(), 8);
	}

	BOOST_AUTO_TEST_CASE(test_no_loop)
	{
		const auto paths = details::make_paths(
				".main:        \n"
				"	jmp @main  \n");

		BOOST_CHECK_EQUAL(paths.size(), 1);
	}

	BOOST_AUTO_TEST_CASE(test_in_range_label)
	{
		const auto paths = details::make_paths(
				".main:              \n"
				"	cls              \n"
				"	cls              \n"
				"                    \n"
				".taken_first_without_jump:\n"
				"	cls              \n"
				"   jmp @taken_first_without_jump\n");

		BOOST_CHECK_EQUAL(paths.size(), 1);
	}

	//BOOST_AUTO_TEST_CASE(test_in_range_label_not_same_alignement)
	// {
	//  // TODO: raw() should optionally produce 16 or 8 bit code
	//
	// 	const auto paths = details::make_paths(
	// 			".main:              \n"
	// 			"	cls              \n"
	// 			"	cls              \n"
	// 			"                    \n"
	// 			".taken_first_without_jump:\n"
	// 			"	cls              \n"
	// 			"   jmp @taken_first_without_jump\n");

	// 	BOOST_CHECK_EQUAL(paths.size(), 1);
	// }



BOOST_AUTO_TEST_SUITE_END()