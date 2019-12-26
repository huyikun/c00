#include "tokenizer/tokenizer.h"

#include <cctype>
#include <sstream>

namespace plc0 {

	std::pair<std::optional<Token>, std::optional<CompilationError>> Tokenizer::NextToken() {
		if (!_initialized)
			readAll();
		if (_rdr.bad())
			return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrorCode::ErrStreamError));
		if (isEOF())
			return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrorCode::ErrEOF));
		auto p = nextToken();
		if (p.second.has_value())
			return std::make_pair(p.first, p.second);
		auto err = checkToken(p.first.value());
		if (err.has_value())
			return std::make_pair(p.first, err.value());
		return std::make_pair(p.first, std::optional<CompilationError>());
	}

	std::pair<std::vector<Token>, std::optional<CompilationError>> Tokenizer::AllTokens() {
		std::vector<Token> result;
		while (true) {
			auto p = NextToken();
			if (p.second.has_value()) {
				if (p.second.value().GetCode() == ErrorCode::ErrEOF)
					return std::make_pair(result, std::optional<CompilationError>());
				else
					return std::make_pair(std::vector<Token>(), p.second);
			}
			result.emplace_back(p.first.value());
		}
	}

	std::pair<std::optional<Token>, std::optional<CompilationError>> Tokenizer::nextToken() {
		std::stringstream ss;
		std::pair<std::optional<Token>, std::optional<CompilationError>> result;
		std::pair<int64_t, int64_t> pos;
		DFAState current_state = DFAState::INITIAL_STATE;
		while (true) {
			auto current_char = nextChar();
			switch (current_state) {
			case INITIAL_STATE: {
				if (!current_char.has_value())
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(0, 0, ErrEOF));
				auto ch = current_char.value();
				auto invalid = false;

				if (plc0::isspace(ch))
					current_state = DFAState::INITIAL_STATE;
				else if (!plc0::isprint(ch))
					invalid = true;
				else if (plc0::isdigit(ch)) {
					if (ch == '0')
						current_state = DFAState::ZERO_STATE;
					else
						current_state = DFAState::DECINTEGER_STATE;
				}
				else if (plc0::isalpha(ch))
					current_state = DFAState::IDENTIFIER_STATE;
				else {
					switch (ch) {
					case '=':
						current_state = DFAState::EQUAL_SIGN_STATE;
						break;
					case '-':
						current_state = DFAState::MINUS_SIGN_STATE;
						break;
					case '+':
						current_state = DFAState::PLUS_SIGN_STATE;
						break;
					case '*':
						current_state = DFAState::MULTIPLICATION_SIGN_STATE;
						break;
					case '/':
						current_state = DFAState::DIVISION_SIGN_STATE;
						break;
					case ';':
						current_state = DFAState::SEMICOLON_STATE;
						break;
					case '(':
						current_state = DFAState::LEFTBRACKET_STATE;
						break;
					case ')':
						current_state = DFAState::RIGHTBRACKET_STATE;
						break;
					case '{':
						current_state = DFAState::LEFTBRACE_STATE;
						break;
					case '}':
						current_state = DFAState::RIGHTBRACE_STATE;
						break;
					case '!':
						current_state = DFAState::EXCLAMATION_STATE;
						break;
					case '<':
						current_state = DFAState::LESSTHAN_STATE;
						break;
					case '>':
						current_state = DFAState::GREATERTHAN_STATE;
						break;
					case ',':
						current_state = DFAState::COMMA_SIGN_STATE;
						break;
					default:
						invalid = true;
						break;
					}
				}
				if (current_state != DFAState::INITIAL_STATE)
					pos = previousPos(); 
				if (invalid) {
					unreadLast();
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidInput));
				}
				if (current_state != DFAState::INITIAL_STATE)
					ss << ch;
				break;
			}

			// 当前状态是无符号整数
			case ZERO_STATE: {
				if (!current_char.has_value()) {
					std::string tmp;
					ss >> tmp;
					return std::make_pair(std::make_optional<Token>(TokenType::HEXINTEGER, tmp, pos, currentPos()), std::optional<CompilationError>());
				}
				auto ch = current_char.value();
				if (ch == 'x' || ch == 'X') {
					current_state = DFAState::HEXINTEGER_STATE;
					ss << ch;
				}
				else if (ch != 'x' && ch != 'X' && plc0::isalpha(ch))
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidIdentifier));
				else if (plc0::isdigit(ch))
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidDecInteger));
				// else if 浮点数处理
				else {
					unreadLast();
					std::string tmp;
					ss >> tmp;
					return std::make_pair(std::make_optional<Token>(TokenType::DECINTEGER, tmp, pos, currentPos()), std::optional<CompilationError>());
				}
				
				break;
			}
			case HEXINTEGER_STATE: {
				if (!current_char.has_value()) {
					std::string tmp;
					ss >> tmp;
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos,ErrorCode::ErrInvalidHexInteger));
				}
				auto ch = current_char.value();
				if (plc0::ishex(ch)) {
					ss << ch;
					break;
				}
				if (plc0::isalpha(ch))	// 其他字母
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidHexInteger));
				else {
					unreadLast();
					std::string tmp;
					ss >> tmp;
					return std::make_pair(std::make_optional<Token>(TokenType::HEXINTEGER, tmp, pos, currentPos()), std::optional<CompilationError>());
				}
				break;
			}
			case DECINTEGER_STATE: {
				if (!current_char.has_value()) {
					std::string tmp;
					ss >> tmp;
					return std::make_pair(std::make_optional<Token>(TokenType::DECINTEGER, tmp, pos, currentPos()), std::optional<CompilationError>());
				}
				auto ch = current_char.value();
				if (plc0::isdigit(ch))
					ss << ch;
				else if(plc0::isalpha(ch))
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidIdentifier));
				else {
					unreadLast();
					std::string tmp;
					ss >> tmp;
					return std::make_pair(std::make_optional<Token>(TokenType::DECINTEGER, tmp, pos, currentPos()), std::optional<CompilationError>());
				}
				break;
			}
			case IDENTIFIER_STATE: {
				if (!current_char.has_value()) {
					std::string tmp;
					ss >> tmp;
					return checkIdentifier(tmp, pos, currentPos());
				}
				auto ch = current_char.value();
				if (plc0::isalpha(ch) || plc0::isdigit(ch))
					ss << ch;
				else
				{
					unreadLast();
					std::string tmp;
					ss >> tmp;
					return checkIdentifier(tmp, pos, currentPos());
				}
				break;
			}
			case PLUS_SIGN_STATE: {
				unreadLast();
				std::string tmp(1, '+');
				return std::make_pair(std::make_optional<Token>(TokenType::PLUS_SIGN, tmp, pos, currentPos()), std::optional<CompilationError>());
			}
			case MINUS_SIGN_STATE: {
				unreadLast();
				std::string tmp(1, '-');
				return std::make_pair(std::make_optional<Token>(TokenType::MINUS_SIGN, tmp, pos, currentPos()), std::optional<CompilationError>());
			}
			case MULTIPLICATION_SIGN_STATE: {
				unreadLast();
				std::string tmp(1, '*');
				return std::make_pair(std::make_optional<Token>(TokenType::MULTIPLICATION_SIGN, tmp, pos, currentPos()), std::optional<CompilationError>());
			}
			case DIVISION_SIGN_STATE: {
				unreadLast();
				std::string tmp(1, '/');
				return std::make_pair(std::make_optional<Token>(TokenType::DIVISION_SIGN, tmp, pos, currentPos()), std::optional<CompilationError>());
			}
			case COMMA_SIGN_STATE: {
				unreadLast();
				std::string tmp(1, ',');
				return std::make_pair(std::make_optional<Token>(TokenType::COMMA_SIGN, tmp, pos, currentPos()), std::optional<CompilationError>());
			}
			case SEMICOLON_STATE: {
				unreadLast();
				std::string tmp(1, ';');
				return std::make_pair(std::make_optional<Token>(TokenType::SEMICOLON, tmp, pos, currentPos()), std::optional<CompilationError>());
			}
			case LEFTBRACKET_STATE: {
				unreadLast();
				std::string tmp(1, '(');
				return std::make_pair(std::make_optional<Token>(TokenType::LEFT_BRACKET, tmp, pos, currentPos()), std::optional<CompilationError>());
			}
			case RIGHTBRACKET_STATE: {
				unreadLast();
				std::string tmp(1, ')');
				return std::make_pair(std::make_optional<Token>(TokenType::RIGHT_BRACKET, tmp, pos, currentPos()), std::optional<CompilationError>());
			}
			case LEFTBRACE_STATE: {
				unreadLast();
				std::string tmp(1, '{');
				return std::make_pair(std::make_optional<Token>(TokenType::LEFT_BRACE, tmp, pos, currentPos()), std::optional<CompilationError>());
			}
			case RIGHTBRACE_STATE: {
				unreadLast();
				std::string tmp(1, '}');
				return std::make_pair(std::make_optional<Token>(TokenType::RIGHT_BRACE, tmp, pos, currentPos()), std::optional<CompilationError>());
			}
			case LESSTHAN_STATE: {
				if (!current_char.has_value()) {
					unreadLast();
					std::string tmp = "<";
					return std::make_pair(std::make_optional<Token>(TokenType::LESSTHAN_SIGN, tmp, pos, currentPos()), std::optional<CompilationError>());
				}
				auto ch = current_char.value();
				if (ch == '=') {
					std::string tmp = "<=";
					return std::make_pair(std::make_optional<Token>(TokenType::LESSTHANOREQUAL_SIGN, tmp, pos, currentPos()), std::optional<CompilationError>());
				}
				else {
					unreadLast();
					std::string tmp = "<";
					return std::make_pair(std::make_optional<Token>(TokenType::LESSTHAN_SIGN, tmp, pos, currentPos()), std::optional<CompilationError>());
				}
			}
			case GREATERTHAN_STATE: {
				if (!current_char.has_value()) {
					unreadLast();
					std::string tmp = ">";
					return std::make_pair(std::make_optional<Token>(TokenType::GREATERTHAN_SIGN, tmp, pos, currentPos()), std::optional<CompilationError>());
				}
				auto ch = current_char.value();
				if (ch == '=') {
					std::string tmp = ">=";
					return std::make_pair(std::make_optional<Token>(TokenType::GREATERTHANOREQUAL_SIGN, tmp, pos, currentPos()), std::optional<CompilationError>());
				}
				else {
					unreadLast();
					std::string tmp = ">";
					return std::make_pair(std::make_optional<Token>(TokenType::GREATERTHAN_SIGN, tmp, pos, currentPos()), std::optional<CompilationError>());
				}
			}
			case EXCLAMATION_STATE: {
				if (!current_char.has_value()) {
					unreadLast();
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidInput));
				}
				auto ch = current_char.value();
				if (ch == '=') {
					std::string tmp = "!=";
					return std::make_pair(std::make_optional<Token>(TokenType::NOTEQUAL_SIGN, tmp, pos, currentPos()), std::optional<CompilationError>());
				}
				else {
					unreadLast();
					return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(pos, ErrorCode::ErrInvalidInput));
				}
			}
			case EQUAL_SIGN_STATE: {
				if (!current_char.has_value()) {
					unreadLast();
					std::string tmp = "=";
					return std::make_pair(std::make_optional<Token>(TokenType::EQUAL_SIGN, tmp, pos, currentPos()), std::optional<CompilationError>());
				}
				auto ch = current_char.value();
				if (ch == '=') {
					std::string tmp = "==";
					return std::make_pair(std::make_optional<Token>(TokenType::DOUBLE_EQUAL, tmp, pos, currentPos()), std::optional<CompilationError>());
				}
				else {
					unreadLast();
					std::string tmp = "=";
					return std::make_pair(std::make_optional<Token>(TokenType::EQUAL_SIGN, tmp, pos, currentPos()), std::optional<CompilationError>());
				}
			}
			default:
				DieAndPrint("unhandled state.");
				break;
			}
		}
		// 预料之外的状态，如果执行到了这里，说明程序异常
		return std::make_pair(std::optional<Token>(), std::optional<CompilationError>());
	}

	std::pair<std::optional<Token>, std::optional<CompilationError>> Tokenizer::checkIdentifier(const std::string& s,std::pair<int64_t,int64_t>p1, std::pair<int64_t, int64_t>p2) {
		auto tmp = std::make_optional<Token>(TokenType::IDENTIFIER, s, p1, p2);
		if (s == "const")
			tmp->SetTokenType(TokenType::CONST);
		else if (s == "void")
			tmp->SetTokenType(TokenType::VOID);
		else if (s == "int")
			tmp->SetTokenType(TokenType::INT);
		else if (s == "char")
			tmp->SetTokenType(TokenType::CHAR);
		else if (s == "double")
			tmp->SetTokenType(TokenType::DOUBLE);
		else if (s == "struct")
			tmp->SetTokenType(TokenType::STRUCT);
		else if (s == "if")
			tmp->SetTokenType(TokenType::IF);
		else if (s == "else")
			tmp->SetTokenType(TokenType::ELSE);
		else if (s == "switch")
			tmp->SetTokenType(TokenType::SWITCH);
		else if (s == "case")
			tmp->SetTokenType(TokenType::CASE);
		else if (s == "default")
			tmp->SetTokenType(TokenType::DEFAULT);
		else if (s == "while")
			tmp->SetTokenType(TokenType::WHILE);
		else if (s == "for")
			tmp->SetTokenType(TokenType::FOR);
		else if (s == "do")
			tmp->SetTokenType(TokenType::DO);
		else if (s == "return")
			tmp->SetTokenType(TokenType::RETURN);
		else if (s == "break")
			tmp->SetTokenType(TokenType::BREAK);
		else if (s == "continue")
			tmp->SetTokenType(TokenType::CONTINUE);
		else if (s == "print")
			tmp->SetTokenType(TokenType::PRINT);
		else if (s == "scan")
			tmp->SetTokenType(TokenType::SCAN);
		//if (tmp->GetType() == TokenType::NULL_TOKEN)
		//	return std::make_pair(std::optional<Token>(), std::make_optional<CompilationError>(p1, ErrorCode::ErrInvalidInput));
		return std::make_pair(tmp, std::optional<CompilationError>());
	}

	std::optional<CompilationError> Tokenizer::checkToken(const Token& t) {
		switch (t.GetType()) {
			case IDENTIFIER: {
				auto val = t.GetValueString();
				if (plc0::isdigit(val[0]))
					return std::make_optional<CompilationError>(t.GetStartPos().first, t.GetStartPos().second, ErrorCode::ErrInvalidIdentifier);
				break;
			}
		default:
			break;
		}
		return {};
	}

	void Tokenizer::readAll() {
		if (_initialized)
			return;
		for (std::string tp; std::getline(_rdr, tp);)
			_lines_buffer.emplace_back(std::move(tp + "\n"));
		_initialized = true;
		_ptr = std::make_pair<int64_t, int64_t>(0, 0);
		return;
	}

	// Note: We allow this function to return a postion which is out of bound according to the design like std::vector::end().
	std::pair<uint64_t, uint64_t> Tokenizer::nextPos() {
		if (_ptr.first >= _lines_buffer.size())
			DieAndPrint("advance after EOF");
		if (_ptr.second == _lines_buffer[_ptr.first].size() - 1)
			return std::make_pair(_ptr.first + 1, 0);
		else
			return std::make_pair(_ptr.first, _ptr.second + 1);
	}

	std::pair<uint64_t, uint64_t> Tokenizer::currentPos() {
		return _ptr;
	}

	std::pair<uint64_t, uint64_t> Tokenizer::previousPos() {
		if (_ptr.first == 0 && _ptr.second == 0)
			DieAndPrint("previous position from beginning");
		if (_ptr.second == 0)
			return std::make_pair(_ptr.first - 1, _lines_buffer[_ptr.first - 1].size() - 1);
		else
			return std::make_pair(_ptr.first, _ptr.second - 1);
	}

	std::optional<char> Tokenizer::nextChar() {
		if (isEOF())
			return {}; // EOF
		auto result = _lines_buffer[_ptr.first][_ptr.second];
		_ptr = nextPos();
		return result;
	}

	bool Tokenizer::isEOF() {
		return _ptr.first >= _lines_buffer.size();
	}

	// Note: Is it evil to unread a buffer?
	void Tokenizer::unreadLast() {
		_ptr = previousPos();
	}


}