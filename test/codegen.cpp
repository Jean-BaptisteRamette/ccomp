#include <boost/test/unit_test.hpp>
#include <ccomp/lexer.hpp>
#include <ccomp/parser.hpp>


BOOST_AUTO_TEST_SUITE(machine_code_generation)

	using namespace ccomp;

	namespace details
	{
		std::vector<arch::opcode>
		try_codegen(std::string&& program)
		{
			auto lex = lexer(std::move(program));
			auto par = parser(lex.enumerate_tokens());
			auto ast = par.make_tree();

			return ast.generate();
		}

		arch::opcode opcode(std::string&& instruction_str)
		{
			std::string program = ".main: " + instruction_str;
			return try_codegen(std::move(program))[1];
		}
	}

#define BOOST_CHECK_EQUAL_RANGES(Rng1, Rng2) BOOST_CHECK_EQUAL_COLLECTIONS(Rng1.begin(), Rng1.end(), Rng2.begin(), Rng2.end());

	BOOST_AUTO_TEST_CASE(check_per_instruction_opcodes)
	{
		BOOST_CHECK_EQUAL(details::opcode("mov ra, 0x69"), 0x6A69);
		BOOST_CHECK_EQUAL(details::opcode("mov ra, rb"),   0x8AB0);
		BOOST_CHECK_EQUAL(details::opcode("mov dt, re"),   0xFE15);
		BOOST_CHECK_EQUAL(details::opcode("mov st, re"),   0xFE18);
		BOOST_CHECK_EQUAL(details::opcode("mov r4, dt"),   0xF407);

		BOOST_CHECK_EQUAL(details::opcode("ret"), 0x00EE);
		BOOST_CHECK_EQUAL(details::opcode("cls"), 0x00E0);

		BOOST_CHECK_EQUAL(details::opcode("se r7, r8"),  0x5780);
		BOOST_CHECK_EQUAL(details::opcode("sne r7, r8"), 0x9780);

		BOOST_CHECK_EQUAL(details::opcode("add rf, rf"),   0x8FF4);
		BOOST_CHECK_EQUAL(details::opcode("add rf, 0x22"), 0x7F22);
		BOOST_CHECK_EQUAL(details::opcode("add ar, rd"),   0xFD1E);
		BOOST_CHECK_EQUAL(details::opcode("inc rd"),       0x7D01);

		BOOST_CHECK_EQUAL(details::opcode("sub r1, r2"),  0x8125);
		BOOST_CHECK_EQUAL(details::opcode("suba r4, rf"), 0x84F7);

		BOOST_CHECK_EQUAL(details::opcode("or  r2, r3"), 0x8231);
		BOOST_CHECK_EQUAL(details::opcode("and r2, r3"), 0x8232);
		BOOST_CHECK_EQUAL(details::opcode("xor r2, r3"), 0x8233);
		BOOST_CHECK_EQUAL(details::opcode("shl r2, r3"), 0x823E);
		BOOST_CHECK_EQUAL(details::opcode("shl r2"),     0x820E);
		BOOST_CHECK_EQUAL(details::opcode("shr r2"),     0x8206);
		BOOST_CHECK_EQUAL(details::opcode("shr r2, r3"), 0x8236);

		BOOST_CHECK_EQUAL(details::opcode("rdump r2"), 0xF255);
		BOOST_CHECK_EQUAL(details::opcode("rload r2"), 0xF265);

		BOOST_CHECK_EQUAL(details::opcode("rand r2, 0x70"), 0xC270);
		BOOST_CHECK_EQUAL(details::opcode("bcd r4"), 0xF433);

		BOOST_CHECK_EQUAL(details::opcode("wkey r9"), 0xF90A);
		BOOST_CHECK_EQUAL(details::opcode("ske rd"), 0xED9E);
		BOOST_CHECK_EQUAL(details::opcode("skne rd"), 0xEDA1);
		BOOST_CHECK_EQUAL(details::opcode("draw rd, re, 0xF"), 0xDDEF);

		BOOST_CHECK_EQUAL(details::opcode("jmp [0xFFF]"), 0xBFFF);
	}

	BOOST_AUTO_TEST_CASE(check_raw_statements)
	{
		const auto code = details::try_codegen(".main:                     \n"
											   "    define opcode 0xFF'FF  \n"
											   "    raw(0x0000)            \n"
											   "    raw(1)                 \n"
											   "    raw(opcode)            \n"
											   "    raw(opcode)            \n");

		const auto expected_code = {
				0x0000,
				0x0001,
				0xFF'FF,
				0xFF'FF
		};

		BOOST_CHECK_EQUAL_RANGES(code, expected_code);
	}

	BOOST_AUTO_TEST_CASE(check_address)
	{
		const auto code = details::try_codegen("proc a            \n"   // 0x06
											   "    xor r0, r0    \n"   // 0x06
											   "    xor r0, r0    \n"   // 0x08
											   "    xor r0, r0    \n"   // 0x0A
											   "    xor r0, r0    \n"   // 0x0C
											   "    xor r0, r0    \n"   // 0x0E
											   "    xor r0, r0    \n"   // 0x10
											   "    xor r0, r0    \n"   // 0x12
											   "    xor r0, r0    \n"   // 0x14
											   "    xor r0, r0    \n"   // 0x16
											   "    xor r0, r0    \n"   // 0x18
											   "    xor r0, r0    \n"   // 0x1A
											   "    xor r0, r0    \n"   // 0x1C
											   "    xor r0, r0    \n"   // 0x1E
											   "    call $b       \n"   // 0x20
											   "    ret           \n"   // 0x22
											   "endp a            \n"
											   "                  \n"
											   "proc b            \n"   // 0x24
											   "    xor r1, r1    \n"   // 0x24
											   "    xor r1, r1    \n"   // 0x26
											   "    xor r1, r1    \n"   // 0x28
											   "    xor r1, r1    \n"   // 0x2A
											   "    xor r1, r1    \n"   // 0x2C
											   "    xor r1, r1    \n"   // 0x2E
											   "    xor r1, r1    \n"   // 0x30
											   "    xor r1, r1    \n"   // 0x32
											   "    xor r1, r1    \n"   // 0x34
											   "    xor r1, r1    \n"   // 0x36
											   "    xor r1, r1    \n"   // 0x38
											   "    xor r1, r1    \n"   // 0x3A
											   "    xor r1, r1    \n"   // 0x3C
											   "    call $a       \n"   // 0x3E
											   "    jmp @done     \n"   // 0x40
											   ".done:            \n"
											   "    ret           \n"   // 0x42
											   "endp b            \n"
											   "                  \n"
											   "                  \n"
											   ".main:            \n"   // 0x00
											   "    call $a       \n"   // 0x00
											   "    call $b       \n"   // 0x02
											   "    jmp @main     \n"); // 0x04

		const auto expected_code = {
				0x2006,
				0x2024,
				0x1000,

				0x8003,
				0x8003,
				0x8003,
				0x8003,
				0x8003,
				0x8003,
				0x8003,
				0x8003,
				0x8003,
				0x8003,
				0x8003,
				0x8003,
				0x8003,

				0x2024,
				0x00EE,

				0x8113,
				0x8113,
				0x8113,
				0x8113,
				0x8113,
				0x8113,
				0x8113,
				0x8113,
				0x8113,
				0x8113,
				0x8113,
				0x8113,
				0x8113,

				0x2006,
				0x1042,
				0x00EE
		};

		BOOST_CHECK_EQUAL_RANGES(code, expected_code);
	}

	BOOST_AUTO_TEST_CASE(check_pseudo_instruction_swap)
	{
		const auto code = details::try_codegen(".main:\n swp r0, r1");
		const auto expected_code = { 0x8013, 0x8103, 0x8013 };

		BOOST_CHECK_EQUAL_RANGES(code, expected_code);
	}

#undef BOOST_RANGE_EQUAL

BOOST_AUTO_TEST_SUITE_END()