#pragma once

#include "error/error.h"
#include "instruction/instruction.h"
#include "tokenizer/token.h"
#include "analyser/table.h"

#include <vector>
#include <set>
#include <optional>
#include <utility>
#include <map>
#include <cstdint>
#include <cstddef>		// for std::size_t
#define MAX_FUNCS 65535 // 0xffff
namespace plc0
{
class Analyser final
{
private:
	using uint64_t = std::uint64_t;
	using int64_t = std::int64_t;
	using uint32_t = std::uint32_t;
	using int32_t = std::int32_t;

public:
	Analyser(std::vector<Token> v)
		: _tokens(std::move(v)), _offset(0), _instructions({}), _current_pos(0, 0),
		  _var({}), _start({}), _fun({}), _indexTable({}), _nextTokenIndex(0),
		  _nextVarAddress(0), _instructionIndex(-1) {}
	Analyser(Analyser &&) = delete;
	Analyser(const Analyser &) = delete;
	Analyser &operator=(Analyser) = delete;

	std::pair<std::vector<functionInstructions>, std::optional<CompilationError>> Analyse();
	std::vector<Instruction> getStartCode();
	std::vector<varTable> getVarTable();
	std::vector<functionsTable> getFunctionTable();

private:
	// 所有的递归子程序

	// <程序>
	std::optional<CompilationError> analyseProgram();
	// <variable - declaration>
	std::optional<CompilationError> analyseVariableDeclaration(bool isGlobal);
	// <function - definition>
	std::optional<CompilationError> analyseFunctionDefinition();
	// <全局变量>
	std::optional<CompilationError> analyseParamClause();
	// <init-declarator-list>
	std::optional<CompilationError> analyseInitDeclarator(bool isConstant, bool isGlobal);
	// <param-declaration-list>
	std::optional<CompilationError> analyseParamDeclarationList();
	// <param-declaration>
	std::optional<CompilationError> analyseParamDeclaration();
	// <语句序列>
	std::optional<CompilationError> analyseCompoundStatement();

	std::optional<CompilationError> analyseStatementseq();

	// <语句集>
	std::optional<CompilationError> analyseStatement();

	std::optional<CompilationError> analyseConditionStatement();
	std::optional<CompilationError> analyseCondition();
	std::optional<CompilationError> analyseAssignmentStatement();
	std::optional<CompilationError> analyseJumpStatement();
	std::optional<CompilationError> analysePrintStatement();
	std::optional<CompilationError> analysePrintableList();
	std::optional<CompilationError> analysePrintable();
	std::optional<CompilationError> analyseScanStatement();
	std::optional<CompilationError> analyseLoopStatement();
	std::optional<CompilationError> analyseFunctionCall();

	// <表达式>
	std::optional<CompilationError> analyseExpression();
	std::optional<CompilationError> analyseMultiExpression();
	std::optional<CompilationError> analyseUnaryExpression();
	std::optional<CompilationError> analysePrimaryExpression();

	// Token 缓冲区相关操作
	// 返回下一个 token
	std::optional<Token> nextToken();
	// 回退一个 token
	void unreadToken();

	// 下面是符号表相关操作

	// helper function
	void _add(const Token &, int32_t type, int32_t level);
	// 添加变量、常量、未初始化的变量
	void addVariable(const Token &, int32_t level);
	void addConstant(const Token &, int32_t level);
	void addUninitializedVariable(const Token &, int32_t level);
	void addFunction(const Token &, int32_t level);

	//void updateUninitializedVar(const std::string&);

	// 是否被声明过
	bool isDeclared(const std::string &, int32_t level);
	//bool isDeclaredFunction(const std::string& s);
	bool isFunctionName(const std::string &s);
	// 是否是未初始化的变量
	bool isUninitializedVariable(const std::string &, int32_t level);
	// 是否是已初始化的变量
	bool isInitializedVariable(const std::string &, int32_t level);
	// 是否是常量
	bool isConstant(const std::string &, int32_t level);
	// 获得 {变量，常量} 在栈上的偏移
	int32_t getIndex(const std::string &, int32_t level);

	// 自定义
	int32_t isOverflow(const std::string &);
	bool isTypeSpecifier(const Token &tk);
	bool isPlusOrMinus(const Token &tk);
	bool isMultiOrDevide(const Token &tk);
	bool isRelationalOperator(const Token &tk);

private:
	std::vector<Token> _tokens;
	std::size_t _offset;
	std::vector<Instruction> _instructions;
	std::pair<uint64_t, uint64_t> _current_pos;


	std::vector<varTable> _var;
	std::vector<Instruction> _start;
	std::vector<functionsTable> _fun;
	std::vector<std::vector<Instruction>> _fun_body;
	std::vector<functionInstructions> _funcInstructions;

	std::vector<int32_t> _indexTable;

	int32_t _nextTokenIndex;
	// 符号在栈上的偏移
	int32_t _nextVarAddress;
	//函数指令集的索引 （-1 ：全局初始化
	int32_t _instructionIndex;
};

} 