#include "fmt/core.h"
#include "tokenizer/tokenizer.h"
#include "analyser/analyser.h"

namespace fmt {
	template<>
	struct formatter<plc0::ErrorCode> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const plc0::ErrorCode &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
				case plc0::ErrNoError:
                    name = "No error.";
                    break;
                case plc0::ErrStreamError:
                    name = "Stream error.";
                    break;
                case plc0::ErrEOF:
                    name = "EOF.";
                    break;
                case plc0::ErrInvalidInput:
                    name = "The input is invalid.";
                    break;
                case plc0::ErrInvalidIdentifier:
                    name = "Identifier is invalid.";
                    break;
                case plc0::ErrIntegerOverflow:
                    name = "The integer is overflow.";
                    break;
                case plc0::ErrNeedIdentifier:
                    name = "Need an identifier here.";
                    break;
				case plc0::ErrNeedTypeSpecifier:
					name = "Need an type specifier here.";
					break; 
				case plc0::ErrNeedRightBracket:
					name = "Need an right bracket here.";
					break;
				case plc0::ErrNeedLeftBrace:
					name = "Need an left brace here.";
					break;
				case plc0::ErrNeedLeftBracket:
					name = "Need an Left Bracket here.";
					break;
				case plc0::ErrNeedRightBrace:
					name = "Need an right brace here.";
					break;
				case plc0::ErrInvalidParam:
					name = "Invalid param here.";
					break;
                case plc0::ErrConstantNeedValue:
                    name = "The constant need a value to initialize.";
                    break;
                case plc0::ErrNoSemicolon:
                    name = "Zai? Wei shen me bu xie fen hao.";
                    break;
                case plc0::ErrNoComma:
                    name = "Wei shen me bu xie dou hao.";
                    break;
                case plc0::ErrNoLeftBracket:
                    name = "Wei shen me bu xie zuo kuo hao.";
                    break;
			    case plc0::ErrNoRightBracket:
                    name = "Wei shen me bu xie you kuo hao.";
                    break;
                case plc0::ErrNoLeftBrace:
                    name = "Wei shen me bu xie zuo dadada kuo hao.";
                    break;
                case plc0::ErrNoRightBrace:
                    name = "Wei shen me bu xie you dadada kuo hao.";
                    break;
                case plc0::ErrInvalidVariableDeclaration:
                    name = "The variable declaration is invalid.";
                    break;
				case plc0::ErrInvalidExpression:
					name = "The expression is invalid.";
					break;
				case plc0::ErrInvalidStatement:
					name = "The statement is invalid.";
					break;
				case plc0::ErrInvalidScan:
					name = "The scan statment is invalid.";
					break;
				case plc0::ErrInvalidDecInteger:
					name = "The dec integer is invalid.";
					break;
				case plc0::ErrInvalidHexInteger:
					name = "The hex integer is invalid.";
					break;
				case plc0::ErrInvalidFunctionDefinition:
					name = "The function definition is invalid.";
					break;
				case plc0::ErrNeedMainFunction:
					name = "The program need a main.";
					break;
				case plc0::ErrIncompleteExpression:
					name = "The expression is incomplete.";
					break;
                case plc0::ErrIncompleteStatement:
                    name = "The statement is incomplete.";
                    break;
                case plc0::ErrIncompleteCondition:
                    name = "The condition is incomplete.";
                    break;
			    case plc0::ErrIncompleteFunctionCall:
                    name = "The function call is incomplete.";
                    break;
                case plc0::ErrNeedReturnValue:
                    name = "The function need return value.";
                    break;
                case plc0::ErrNoNeedReturnValue:
                    name = "The function does not have return value.";
                    break;
                case plc0::ErrNotDeclared:
                    name = "The variable or constant must be declared before being used.";
                    break;
                case plc0::ErrAssignToConstant:
                    name = "Trying to assign value to a constant.";
                    break;
                case plc0::ErrDuplicateDeclaration:
                    name = "The variable or constant has been declared.";
                    break;
                case plc0::ErrNotInitialized:
                    name = "The variable has not been initialized.";
                    break;
                case plc0::ErrInvalidAssignment:
                    name = "The assignment statement is invalid.";
                    break;
                case plc0::ErrInvalidPrint:
                    name = "The output statement is invalid.";
                    break;
                case plc0::ErrInvalidOperator:
                    name = "The operator is invalid.";
                    break;
                case plc0::ErrInvalidIntegerLiteral:
                    name = "The integer-literal is invalid.";
                    break;
                case plc0::ErrAnnotationUnmatched:
                    name = "The annotation is unmatched.";
                    break;
			}
			return format_to(ctx.out(), name);
		}
	};

	template<>
	struct formatter<plc0::CompilationError> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const plc0::CompilationError &p, FormatContext &ctx) {
			return format_to(ctx.out(), "Line: {} Column: {} Error: {}", p.GetPos().first, p.GetPos().second, p.GetCode());
		}
	};
}

namespace fmt {
	template<>
	struct formatter<plc0::Token> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const plc0::Token &p, FormatContext &ctx) {
			return format_to(ctx.out(),
				"Line:{}\t Column:{}\t Type:{}\t\t Value:{}",
				p.GetStartPos().first, p.GetStartPos().second, p.GetType(), p.GetValueString());
		}
	};

	template<>
	struct formatter<plc0::TokenType> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const plc0::TokenType &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
			case plc0::NULL_TOKEN:
				name = "NullToken";
				break;
			case plc0::HEXINTEGER:
				name = "HEXINTEGER";
				break;
			case plc0::IDENTIFIER:
				name = "Identifier";
				break;
			case plc0::DECINTEGER:
				name = "DECINTEGER";
				break;
			case plc0::DOUBLE_EQUAL:
				name = "DOUBLE_EQUAL";
				break;
			case plc0::NOTEQUAL_SIGN:
				name = "NOTEQUAL_SIGN";
				break;
			case plc0::LESSTHAN_SIGN:
				name = "LESSTHAN_SIGN";
				break;
			case plc0::GREATERTHAN_SIGN:
				name = "GREATERTHAN_SIGN";
				break;
			case plc0::PLUS_SIGN:
				name = "PlusSign";
				break;
			case plc0::MINUS_SIGN:
				name = "MinusSign";
				break;
			case plc0::MULTIPLICATION_SIGN:
				name = "MultiplicationSign";
				break;
			case plc0::DIVISION_SIGN:
				name = "DivisionSign";
				break;
			case plc0::EQUAL_SIGN:
				name = "EqualSign";
				break;
			case plc0::SEMICOLON:
				name = "Semicolon";
				break;
			case plc0::LEFT_BRACKET:
				name = "LeftBracket";
				break;
			case plc0::RIGHT_BRACKET:
				name = "RightBracket";
				break;
			case plc0::LEFT_BRACE:
				name = "LEFT_BRACE";
				break;
			case plc0::RIGHT_BRACE:
				name = "RIGHT_BRACE";
				break;
			case plc0::LESSTHANOREQUAL_SIGN:
				name = "LESSTHANOREQUAL_SIGN";
				break;
			case plc0::GREATERTHANOREQUAL_SIGN:
				name = "GREATERTHANOREQUAL_SIGN";
				break;
			case plc0::COMMA_SIGN:
				name = "COMMA_SIGN";
				break;
			case plc0::CONST:
				name = "CONST";
				break;
			case plc0::VOID:
				name = "VOID";
				break;
			case plc0::INT:
				name = "INT";
				break;
			case plc0::CHAR:
				name = "CHAR";
				break;
			case plc0::DOUBLE:
				name = "DOUBLE";
				break;
			case plc0::STRUCT:
				name = "STRUCT";
				break;
			case plc0::IF:
				name = "IF";
				break;
			case plc0::ELSE:
				name = "ELSE";
				break;
			case plc0::SWITCH:
				name = "SWITCH";
				break;
			case plc0::CASE:
				name = "CASE";
				break;
			case plc0::DEFAULT:
				name = "DEFAULT";
				break;
			case plc0::WHILE:
				name = "WHILE";
				break;
			case plc0::FOR:
				name = "FOR";
				break;
			case plc0::DO:
				name = "DO";
				break;
			case plc0::RETURN:
				name = "RETURN";
				break;
			case plc0::BREAK:
				name = "BREAK";
				break;
			case plc0::CONTINUE:
				name = "CONTINUE";
				break;
			case plc0::PRINT:
				name = "PRINT";
				break;
			case plc0::SCAN:
				name = "SCAN";
				break;
			}
			return format_to(ctx.out(), name);
		}
	};
}

namespace fmt {
	template<>
	struct formatter<plc0::Operation> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const plc0::Operation &p, FormatContext &ctx) {
			std::string name;
			switch (p) {
			case plc0::NOP:
				name = "nop";
				break;
			case plc0::BIPUSH:
				name = "bipush";
				break;
			case plc0::IPUSH:
				name = "ipush";
				break;
			case plc0::POP:
				name = "pop";
				break;
			case plc0::POP2:
				name = "pop2";
				break;
			case plc0::POPN:
				name = "popn";
				break;
			case plc0::DUP:
				name = "dup";
				break;
			case plc0::LOADC:
				name = "loadc";
				break;
			case plc0::LOADA:
				name = "loada";
				break;
			case plc0::NEW:
				name = "new";
				break;
			case plc0::SNEW:
				name = "snew";
				break;
			case plc0::ILOAD:
				name = "iload";
				break;
			case plc0::ALOAD:
				name = "aload";
				break;
			case plc0::IALOAD:
				name = "iaload";
				break;
			case plc0::ISTORE:
				name = "istore";
				break;
			case plc0::IASTORE:
				name = "iastore";
				break;
			case plc0::IADD:
				name = "iadd";
				break;
			case plc0::ISUB:
				name = "isub";
				break;
			case plc0::IMUL:
				name = "imul";
				break;
			case plc0::IDIV:
				name = "idiv";
				break;
			case plc0::INEG:
				name = "ineg";
				break;
			case plc0::ICMP:
				name = "icmp";
				break;
			case plc0::JMP:
				name = "jmp";
				break;
			case plc0::JE:
				name = "je";
				break;
			case plc0::JNE:
				name = "jne";
				break;
			case plc0::JL:
				name = "jl";
				break;
			case plc0::JGE:
				name = "jge";
				break;
			case plc0::JG:
				name = "jg";
				break;
			case plc0::JLE:
				name = "jle";
				break;
			case plc0::CALL:
				name = "call";
				break;
			case plc0::RET:
				name = "ret";
				break;
			case plc0::IRET:
				name = "iret";
				break;
			case plc0::IPRINT:
				name = "iprint";
				break;
			case plc0::CPRINT:
				name = "cprint";
				break;
			case plc0::PRINTL:
				name = "printl";
				break;
			case plc0::ISCAN:
				name = "iscan";
				break;
			}
			return format_to(ctx.out(), name);
		}
	};
	template<>
	struct formatter<plc0::Instruction> {
		template <typename ParseContext>
		constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

		template <typename FormatContext>
		auto format(const plc0::Instruction &p, FormatContext &ctx) {
			std::string name;
			switch (p.GetOperation())
			{
			case plc0::NOP:
			case plc0::POP:
			case plc0::POP2:
			case plc0::DUP:
			case plc0::NEW:
			case plc0::ILOAD:
			case plc0::ALOAD:
			case plc0::IALOAD:
			case plc0::ISTORE:
			case plc0::IASTORE:
			case plc0::IADD:
			case plc0::ISUB:
			case plc0::IMUL:
			case plc0::IDIV:
			case plc0::INEG:
			case plc0::ICMP:
			case plc0::RET:
			case plc0::IRET:
			case plc0::IPRINT:
			case plc0::PRINTL:
			case plc0::ISCAN:
				return format_to(ctx.out(), "{}", p.GetOperation());
			case plc0::IPUSH:
			case plc0::POPN:
			case plc0::LOADC:
			case plc0::SNEW:
			case plc0::JMP:
			case plc0::JE:
			case plc0::JNE:
			case plc0::JL:
			case plc0::JGE:
			case plc0::JG:
			case plc0::JLE:
			case plc0::CALL:
				return format_to(ctx.out(), "{} {}", p.GetOperation(), p.GetX());
			case plc0::LOADA:
				return format_to(ctx.out(), "{} {}, {}", p.GetOperation(), p.GetX(), p.GetY());
			}
			return format_to(ctx.out(), "NOP");
		}
	};
}