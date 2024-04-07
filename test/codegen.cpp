#include <boost/test/unit_test.hpp>
#include <ccomp/lexer.hpp>
#include <ccomp/parser.hpp>


using namespace ccomp;

BOOST_AUTO_TEST_SUITE(code_generation)

	namespace details
	{
		std::vector<arch::opcode>
		codegen(std::string&& program)
		{
			auto lex = lexer(std::move(program));
			auto par = parser(lex.enumerate_tokens());
			auto ast = par.make_tree();

			return ast.generate();
		}
	}

	BOOST_AUTO_TEST_CASE(check_invalid_operands)
	{

	}

	BOOST_AUTO_TEST_CASE(check_raw_statements)
	{
		const auto code = details::codegen(".main:                     \n"
										   "    define opcode 0xFF'FF  \n"
										   "    raw(0x0000)            \n"
										   "    raw(1)                 \n"
										   "    raw(opcode)            \n"
										   "    raw(opcode)            \n");

		const auto expected_code = std::vector<arch::opcode>({
			0x0000,
			0x0001,
			0xFF'FF,
			0xFF'FF
		});

		BOOST_CHECK_EQUAL_COLLECTIONS(code.begin(),
									  code.end(),
									  expected_code.begin(),
									  expected_code.end());
	}

BOOST_AUTO_TEST_SUITE_END()
