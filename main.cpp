#include "argparse.hpp"
#include "fmt/core.h"

#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"
#include "fmts.hpp"

#include <iostream>
#include <fstream>

using u1 = std::uint8_t;
using u2 = std::uint16_t;
using u4 = std::uint32_t;
using u8 = std::uint64_t;

#define U1_MAX UINT8_MAX
#define U2_MAX UINT16_MAX
#define U4_MAX UINT32_MAX
#define U8_MAX UINT64_MAX

using i1 = std::int8_t;
using i2 = std::int16_t;
using i4 = std::int32_t;
using i8 = std::int64_t;

using f4 = float;
using f8 = double;

using slot_t = i4;
using int_t = slot_t;
using double_t = f8;
using addr_t = slot_t;
using char_t = unsigned char;
using str_t = std::string;



std::vector<plc0::Token> _tokenize(std::istream& input) {
	plc0::Tokenizer tkz(input);
	auto p = tkz.AllTokens();
	if (p.second.has_value()) {
		fmt::print(stderr, "Tokenization error: {}\n", p.second.value());
		exit(2);
	}
	return p.first;
}

void Analyse_s0(std::istream& input, std::ostream& output) {
	auto tks = _tokenize(input);
	plc0::Analyser analyser(tks);
	auto p = analyser.Analyse();
	if (p.second.has_value()) {
		fmt::print(stderr, "analysis_s0 error: {}\n", p.second.value());
		exit(2);
	}

	output << ".constants:" << std::endl;
	auto _fu = analyser.getFunctionTable();
	int nfu = _fu.size();
	for (int i = 0; i < nfu; i++) {
		output << i << "  ";
		output << _fu[i]._type << "  ";
		output << "\"" << _fu[i]._value << "\"" << std::endl;
	}

	output << ".start:" << std::endl;
	auto _st = analyser.getStartCode();
	unsigned int ti = 0;
	for (auto& it : _st) {
		output << ti << " " << fmt::format("{}", it) << std::endl;
		ti++;
	}

	output << ".functions:" << std::endl;
	_fu = analyser.getFunctionTable();
	nfu = _fu.size();
	for (int i = 0; i < nfu; i++) {
		output << i << "  ";
		output << i << "  ";
		output << _fu[i]._params_size << "  ";
		output << _fu[i]._level << std::endl;
	}

	auto v = p.first;
	int nv = v.size();
	for (int i = 0; i < nv; i++) {
		output << ".F" << i << ":" << std::endl;
		int j = 0;
		for (auto& it : v[i]._funins) {
			output << j << " " << fmt::format("{}", it) << std::endl;
			j++;
		}
	}

	return;
}



void Analyse_o0(std::istream& input, std::ostream& output){
	/*
	auto tks = _tokenize(input);
	plc0::Analyser analyser(tks);
	auto p = analyser.Analyse();
	if (p.second.has_value()) {
		fmt::print(stderr, "Syntactic analysis error: {}\n", p.second.value());
		exit(2);
	}
	auto v = p.first;


		char bytes[8];
		const auto writeNBytes = [&](void* addr, int count) {
			assert(0 < count && count <= 8);
			char* p = reinterpret_cast<char*>(addr) + (count - 1);
			for (int i = 0; i < count; ++i) {
				bytes[i] = *p--;
			}
			output.write(bytes, count);
		};

		// magic
		output.write("\x43\x30\x3A\x29", 4);
		// version
		output.write("\x00\x00\x00\x01", 4);
		// constants_count
		auto constants = analyser.getFunctionTable();
		u2 constants_count = constants.size();
		writeNBytes(&constants_count, sizeof constants_count);
		// constants
	
		for (auto& constant : constants) {
			//switch (constant._type)
			//{
			/*case vm::Constant::Type::STRING: {
				out.write("\x00", 1);
				std::string v = std::get<vm::str_t>(constant.value);
				vm::u2 len = v.length();
				writeNBytes(&len, sizeof len);
				out.write(v.c_str(), len);
			} break;
			//case strint: {
				output.write("\x01", 1);
				int_t v = atoi(constant._value);
				writeNBytes(&v, sizeof v);
			//} break;
			/*case vm::Constant::Type::DOUBLE: {
				out.write("\x02", 1);
				vm::double_t v = std::get<vm::double_t>(constant.value);
				writeNBytes(&v, sizeof v);
			} break;
			//default: assert(("unexpected error", false)); break;
			//}
		}

		auto to_binary = [&](const std::vector<plc0::Instruction>& v) {
			u2 instructions_count = v.size();
			writeNBytes(&instructions_count, sizeof instructions_count);
			for (auto& ins : v) {
				u1 op = static_cast<u1>(ins.GetOperation());
				writeNBytes(&op, sizeof op);
				if (auto it = paramSizeOfOpCode.find(ins.GetOperation()); it != paramSizeOfOpCode.end()) {
					auto paramSizes = it->second;
					switch (paramSizes[0]) {
						#define CASE(n) case n: { vm::u##n x = ins.x; writeNBytes(&x, n); }
						CASE(1); break;
						CASE(2); break;
						CASE(4); break;
						#undef CASE
					default: assert(("unexpected error", false));
					}
					if (paramSizes.size() == 2) {
						switch (paramSizes[1]) {
#define CASE(n) case n: { vm::u##n y = ins.y; writeNBytes(&y, n); }
							CASE(1); break;
							CASE(2); break;
							CASE(4); break;
#undef CASE
						default: assert(("unexpected error", false));
						}
					}
				}
			}
		};

		// start
		to_binary(start);
		// functions_count
		vm::u2 functions_count = functions.size();
		writeNBytes(&functions_count, sizeof functions_count);
		// functions
		for (auto& fun : functions) {
			vm::u2 v;
			v = fun.nameIndex; writeNBytes(&v, sizeof v);
			v = fun.paramSize; writeNBytes(&v, sizeof v);
			v = fun.level;     writeNBytes(&v, sizeof v);
			to_binary(fun.instructions);
		}
		*/
	/*
	for (auto& it : v)
		output << fmt::format("{}\n", it);
		*/
	return;
}

int main(int argc, char** argv) {
	argparse::ArgumentParser program("plc0");
	program.add_argument("input")
		.help("speicify the file to be compiled.");
	/*
	program.add_argument("-t")
		.default_value(false)
		.implicit_value(true)
		.help("perform tokenization for the input file.");
	program.add_argument("-l")
		.default_value(false)
		.implicit_value(true)
		.help("perform syntactic analysis for the input file.");
	*/
	program.add_argument("-s")
		.default_value(false)
		.implicit_value(true)
		.help("Translate the input C0 source code into a text assembly file.");
	program.add_argument("-c")
		.default_value(false)
		.implicit_value(true)
		.help("Translate the input C0 source code into binary target file.");
	program.add_argument("-o", "--output")
		.required()
		.default_value(std::string("-"))
		.help("specify the output file.");

	try {
		program.parse_args(argc, argv);
	}
	catch (const std::runtime_error& err) {
		fmt::print(stderr, "{}\n\n", err.what());
		program.print_help();
		exit(2);
	}

	auto input_file = program.get<std::string>("input");
	auto output_file = program.get<std::string>("--output");
	std::istream* input;
	std::ostream* output;
	std::ifstream inf;
	std::ofstream outf;
	if (input_file != "-") {
		inf.open(input_file, std::ios::in);
		if (!inf) {
			fmt::print(stderr, "Fail to open {} for reading.\n", input_file);
			exit(2);
		}
		input = &inf;
	}
	else
		input = &std::cin;
	if (output_file != "-") {
		outf.open(output_file, std::ios::out | std::ios::trunc);
		if (!outf) {
			fmt::print(stderr, "Fail to open {} for writing.\n", output_file);
			exit(2);
		}
		output = &outf;
	}
	else
		output = &std::cout;
	if (program["-s"] == true && program["-c"] == true) {
		fmt::print(stderr, "You can only perform tokenization or syntactic analysis at one time.");
		exit(2);
	}
	if (program["-s"] == true) {
		Analyse_s0(*input, *output);
	}
	else if (program["-c"] == true) {
		Analyse_o0(*input, *output);
	}
	else {
		fmt::print(stderr, "You must choose tokenization or syntactic analysis.");
		exit(2);
	}
	exit(0);
}