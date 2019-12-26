#pragma once

#include "tokenizer/token.h"
#include "tokenizer/utils.hpp"
#include "error/error.h"

#include <utility>
#include <optional>
#include <iostream>
#include <cstdint>
#include <memory>
#include <vector>
#include <string>

namespace plc0 {

	class Tokenizer final {
	private:
		using uint64_t = std::uint64_t;

		// 状态机的所有状态
		enum DFAState {
			INITIAL_STATE,
			ZERO_STATE,
			HEXINTEGER_STATE,
			DECINTEGER_STATE,
			IDENTIFIER_STATE,
			PLUS_SIGN_STATE,
			MINUS_SIGN_STATE,
			DIVISION_SIGN_STATE,
			MULTIPLICATION_SIGN_STATE,
			EQUAL_SIGN_STATE,
			SEMICOLON_STATE,
			COMMA_SIGN_STATE,
			LEFTBRACKET_STATE,
			RIGHTBRACKET_STATE,
			LEFTBRACE_STATE,
			RIGHTBRACE_STATE,
			GREATERTHAN_STATE,
			LESSTHAN_STATE,
			EXCLAMATION_STATE,
		};
	public:
		Tokenizer(std::istream& ifs)
			: _rdr(ifs), _initialized(false), _ptr(0, 0),_lines_buffer() {}
		Tokenizer(Tokenizer&& tkz) = delete;
		Tokenizer(const Tokenizer&) = delete;
		Tokenizer& operator=(const Tokenizer&) = delete;

		std::pair<std::optional<Token>, std::optional<CompilationError>> NextToken();
		std::pair<std::vector<Token>, std::optional<CompilationError>> AllTokens();
	private:
		std::optional<CompilationError> checkToken(const Token&);
		std::pair<std::optional<Token>, std::optional<CompilationError>> nextToken();
		std::pair<std::optional<Token>, std::optional<CompilationError>> checkIdentifier(const std::string& s, std::pair<int64_t, int64_t>p1, std::pair<int64_t, int64_t>p2);

		void readAll();
		std::pair<uint64_t, uint64_t> nextPos();
		std::pair<uint64_t, uint64_t> currentPos();
		std::pair<uint64_t, uint64_t> previousPos();
		std::optional<char> nextChar();
		bool isEOF();
		void unreadLast();

	private:
		std::istream& _rdr;
		bool _initialized;
		std::pair<uint64_t, uint64_t> _ptr;
		std::vector<std::string> _lines_buffer;
	};
}