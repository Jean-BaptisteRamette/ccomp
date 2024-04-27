#include <boost/test/unit_test.hpp>
#include <chasm/lexer.hpp>
#include <chasm/parser.hpp>

#include "options_fixture.hpp"


#define BOOST_CHECK_EQUAL_RANGES(Rng1, Rng2) BOOST_CHECK_EQUAL_COLLECTIONS(Rng1.begin(), Rng1.end(), Rng2.begin(), Rng2.end())


namespace details
{
	using namespace chasm;

	std::vector<uint8_t>
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
		const auto bin = try_codegen(std::move(program));
		return (bin[0] << 8) | bin[1];
	}
}

BOOST_FIXTURE_TEST_SUITE(machine_code_generation, test_env::zero_relocate)

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
		BOOST_CHECK_EQUAL(details::opcode("ldf rf"), 0xFF29);
		BOOST_CHECK_EQUAL(details::opcode("ldfs rf"), 0xFF30);
		BOOST_CHECK_EQUAL(details::opcode("saverpl r6"), 0xF675);
		BOOST_CHECK_EQUAL(details::opcode("loadrpl r9"), 0xF985);

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
				0x00, 0x00,
				0x00, 0x01,
				0xFF, 0xFF,
				0xFF, 0xFF
		};

		BOOST_CHECK_EQUAL_RANGES(code, expected_code);
	}

	BOOST_AUTO_TEST_CASE(check_raw_statements_align_config)
	{
		const auto code = details::try_codegen(".main:                            \n"
											   "    define b1 0xFF                \n"
											   "    define b2 0xFF'FF             \n"
											   "                                  \n"
											   "	config RAW_ALIGNED = 0        \n"
											   "    raw(b1)                       \n"
											   "    raw(b2)                       \n"
											   "                                  \n"
											   "	config RAW_ALIGNED = default  \n"
											   "    raw(b1)                       \n"
											   "    raw(b2)                       \n");

		const auto expected_code = {
				// unaligned
				0xFF,
				0xFF, 0xFF,

				// aligned
				0x00, 0xFF,
				0xFF, 0xFF
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
				0x20, 0x06,
				0x20, 0x24,
				0x10, 0x00,

				0x80, 0x03,
				0x80, 0x03,
				0x80, 0x03,
				0x80, 0x03,
				0x80, 0x03,
				0x80, 0x03,
				0x80, 0x03,
				0x80, 0x03,
				0x80, 0x03,
				0x80, 0x03,
				0x80, 0x03,
				0x80, 0x03,
				0x80, 0x03,

				0x20, 0x24,
				0x00, 0xEE,

				0x81, 0x13,
				0x81, 0x13,
				0x81, 0x13,
				0x81, 0x13,
				0x81, 0x13,
				0x81, 0x13,
				0x81, 0x13,
				0x81, 0x13,
				0x81, 0x13,
				0x81, 0x13,
				0x81, 0x13,
				0x81, 0x13,
				0x81, 0x13,

				0x20, 0x06,
				0x10, 0x42,
				0x00, 0xEE
		};

		BOOST_CHECK_EQUAL_RANGES(code, expected_code);
	}

	BOOST_AUTO_TEST_CASE(check_pseudo_instruction_swap)
	{
		const auto code = details::try_codegen(".main:\n swp r0, r1");
		const auto expected_code = {
				0x80, 0x13,
				0x81, 0x03,
				0x80, 0x13
		};

		BOOST_CHECK_EQUAL_RANGES(code, expected_code);
	}

	BOOST_AUTO_TEST_CASE(check_sprite_even_sized)
	{
		const auto code = details::try_codegen("sprite s [1, 2, 3, 4, 5, 6]\n"
											   ".main:                     \n"
											   "    draw r0, r0, #s        \n");

		const auto expected_code = {
				0xD0, 0x06,
				0x01, 0x02,
				0x03, 0x04,
				0x05, 0x06
		};

		BOOST_CHECK_EQUAL_RANGES(code, expected_code);
	}

	BOOST_AUTO_TEST_CASE(check_sprite_odd_sized)
	{
		const auto code = details::try_codegen("sprite s [1, 2, 3, 4, 5]\n"
											   ".main:                  \n"
											   "    draw r0, r0, #s     \n");

		const auto expected_code = {
				0xD0, 0x05,
				0x01, 0x02,
				0x03, 0x04,
				0x05
		};

		BOOST_CHECK_EQUAL_RANGES(code, expected_code);
	}

BOOST_AUTO_TEST_SUITE_END()


BOOST_AUTO_TEST_SUITE(sprite_padding)

	BOOST_FIXTURE_TEST_CASE(padding_off, test_env::default_options)
	{
		const auto code = details::try_codegen("sprite s1 [1, 2, 3]  \n"
											   "sprite s2 [1, 2, 3]  \n"
											   "sprite s3 [1, 2, 3]  \n"
											   ".main:               \n");

		const auto expected_code = {
				1, 2, 3,
				1, 2, 3,
				1, 2, 3
		};

		BOOST_CHECK_EQUAL_RANGES(code, expected_code);
	}

	BOOST_FIXTURE_TEST_CASE(padding_on, test_env::padded_sprites)
	{
		const auto code = details::try_codegen("sprite s1 [1, 2, 3]  \n"
											   "sprite s2 [1, 2, 3]  \n"
											   "sprite s3 [1, 2, 3]  \n"
											   ".main:               \n");

		const auto expected_code = {
				1, 2, 3, 0,
				1, 2, 3, 0,
				1, 2, 3, 0
		};

		BOOST_CHECK_EQUAL_RANGES(code, expected_code);
	}

BOOST_AUTO_TEST_SUITE_END()

#undef BOOST_CHECK_EQUAL_RANGES
