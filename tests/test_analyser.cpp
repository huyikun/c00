
#include "analyser/analyser.h"
#include "instruction/instruction.h"
#include "analyser/table.h"
#include "tokenizer/tokenizer.h"

#include <sstream>
#include <vector>

#include "fmt/core.h"
#include "fmts.hpp"
#include "simple_vm.hpp"
std::ostream& operator<<(std::ostream& os,
	plc0::CompilationError const& t) {
	os << fmt::format("{}", t);
	return os;
}
std::ostream& operator<<(std::ostream& os, plc0::Instruction const& t) {
	os << fmt::format("{}", t);
	return os;
}

// template <typename T>
// struct formatter<std::vector<T>> {
//   template <typename ParseContext>
//   constexpr auto parse(ParseContext& ctx) {
//     return ctx.begin();
//   }

//   template <typename FormatContext>
//   auto format(const std::vector<T>& p, FormatContext& ctx) {
//     auto res = format_to(ctx.out(), "[");
//     for (auto& i : p) format_to(ctx.out(), "{}, ", i);
//     return format_to(ctx.out(), "]");
//   }
// };
#include "catch2/catch.hpp"

std::pair<std::vector<plc0::functionInstructions>, std::optional<plc0::CompilationError>>
	analyze(std::string& input) {
	std::stringstream ss(input);
	plc0::Tokenizer lexer(ss);
	auto tokens = lexer.AllTokens();
	plc0::Analyser parser(tokens.first);
	return parser.Analyse();

}

TEST_CASE("Basic analyzing program") {
	std::string input = 
		"int a = 2;\n"
		"int main(){"
		"	print(a+1);"
		"	return 1;\n"
		"}";
	auto result = analyze(input);
	REQUIRE(result.first.size() == 0);
	REQUIRE_FALSE(result.second.has_value());
}
