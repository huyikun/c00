#include "analyser.h"

#include <climits>

namespace plc0 {

	std::pair<std::vector<functionInstructions>, std::optional<CompilationError>> Analyser::Analyse() {
		auto err = analyseProgram();
		if (err.has_value())
			return std::make_pair(std::vector<functionInstructions>(), err);
		else
			return std::make_pair(_funcInstructions, std::optional<CompilationError>());
	}

	// <C0-program> ::= {<variable - declaration>} {<function - definition>}
	std::optional<CompilationError> Analyser::analyseProgram() {
		_indexTable.emplace_back(0);
		// <variable - declaration>
		auto err = analyseVariableDeclaration(true);
		if (err.has_value()) 
			return err;
		err = analyseFunctionDefinition();
		if (err.has_value())
			return err;
		return {};
	}

	// <variable-declaration> ::= [<const - qualifier>] <type - specifier> < init - declarator - list>';'
	std::optional<CompilationError> Analyser::analyseVariableDeclaration(bool isGlobal) {
		while (true) {
			bool isConst = false;
			auto next = nextToken();
			if (!next.has_value() || (next.value().GetType() != TokenType::CONST && !isTypeSpecifier(next.value()))) {
				unreadToken();
				return {};
			}
			if (next.value().GetType() == TokenType::CONST) {
				isConst = true;
				next = nextToken();
			}
			if (!isTypeSpecifier(next.value())) {
				unreadToken();
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedTypeSpecifier);
			}
			auto type = next.value().GetType();

			next = nextToken();
			next = nextToken();
			unreadToken();
			unreadToken();
			if (next.value().GetType() == TokenType::LEFT_BRACKET) {
				unreadToken();
				return {};
			}
			
			auto err = analyseInitDeclarator(isConst, isGlobal);
			if (err.has_value())
				return err;
			while (true) {
				next = nextToken();
				if (next.value().GetType() == SEMICOLON) {
					break;
				}
				else if (next.value().GetType() == COMMA_SIGN) {
					auto err = analyseInitDeclarator(isConst, isGlobal);
					if (err.has_value())
						return err;
				}
				else {
					unreadToken();
					break;
				}
			}
		}
		return {};
	}

	std::optional<CompilationError> Analyser::analyseInitDeclarator(bool isConstant, bool isGlobal) {
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != IDENTIFIER)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);

		auto tmp = next;

		next = nextToken();

		if (isConstant) {
			if (next.value().GetType() != EQUAL_SIGN)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrConstantNeedValue);
			else {
				if (isGlobal) {
					if (isDeclared(tmp.value().GetValueString(), 0))
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);

					auto err = analyseExpression();
					if (err.has_value())
						return err;
					addConstant(tmp.value(), 0);
				}
				else {
					if (isDeclared(tmp.value().GetValueString(), 1))
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);

					auto err = analyseExpression();
					if (err.has_value())
						return err;
					addConstant(tmp.value(), 1);
				}
			}
		}
		else {
			if (next.value().GetType() == TokenType::COMMA_SIGN || next.value().GetType() == TokenType::SEMICOLON) {//未赋值
				unreadToken();
				if (isGlobal) {
					if (isDeclared(tmp.value().GetValueString(), 0))
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);
					addUninitializedVariable(tmp.value(), 0);
					_start.emplace_back(SNEW, 1, 0);
				}
				else { 
					if (isDeclared(tmp.value().GetValueString(), 1))
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);
					addUninitializedVariable(tmp.value(), 1);
					_funcInstructions[_instructionIndex]._funins.emplace_back(SNEW, 1, 0);
				}
				return {};
			}
			else {
				if (isGlobal) {
					if (isDeclared(tmp.value().GetValueString(), 0))
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);

					auto err = analyseExpression();
					if (err.has_value())
						return err;
					addVariable(tmp.value(), 0);
				}
				else {
					if (isDeclared(tmp.value().GetValueString(), 1))
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);

					addUninitializedVariable(tmp.value(), 1);
					auto err = analyseExpression();
					if (err.has_value())
						return err;
					unsigned int nn = _var.size();
					_var[nn - 1]._type = 2;
				}
			}
		}
		return {};
	}

	std::optional<CompilationError> Analyser::analyseFunctionDefinition() {
		while (true) {
			int slot = 0;
			auto next = nextToken();
			if (!next.has_value())
				return {};
			if (!isTypeSpecifier(next.value())) {
				unreadToken();
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedTypeSpecifier);
			}
			auto type = next;
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);
			if (isDeclared(next.value().GetValueString(), 0))
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrDuplicateDeclaration);
			addFunction(next.value(), 1); 

			_indexTable.emplace_back(_nextVarAddress);
			int oldAddress = _nextVarAddress;

			int tmp = _fun.size();
			_fun[tmp - 1]._haveReturnValue = (type.value().GetType() == TokenType::INT) ? 1 : 0;
			
			/*
			auto err = analyseParamClause();
			if (err.has_value())
				return err;
			*/
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDefinition);
			next = nextToken();
			if (!next.has_value())
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDefinition);
			else if (next.value().GetType() == TokenType::RIGHT_BRACKET) {
				unreadToken();
			}
			else {
				unreadToken();
				auto err = analyseParamDeclaration();
				if (err.has_value())
					return err;
				slot++;
				while (true) {
					next = nextToken();
					if (!next.has_value())
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDefinition);
					if (next.value().GetType() == TokenType::COMMA_SIGN) {
						auto err = analyseParamDeclaration();
						if (err.has_value())
							return err;
						slot++;
					}
					else if (next.value().GetType() == TokenType::RIGHT_BRACKET) {
						unreadToken();
						break;
					}
					else 
						return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDefinition);
				}
			}
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBracket);

			int n = _fun.size();
			_fun[n - 1]._params_size = slot;
			_funcInstructions.emplace_back(functionInstructions());


			auto err = analyseCompoundStatement();
			if (err.has_value())
				return err;

			int nvar = _var.size();
			while (nvar > oldAddress) {
				_var.pop_back();
				nvar = _var.size();
			}
			_indexTable[1] = oldAddress;
			_nextVarAddress = oldAddress;
		}
	}
	/*
	std::optional<CompilationError> Analyser::analyseParamClause() {
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDefinition);
		next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDefinition);
		else if (next.value().GetType() == TokenType::RIGHT_BRACKET) {
			unreadToken();
		}
		else {
			unreadToken();
			auto err = analyseParameterDeclaration();
			if (err.has_value())
				return err;
			slot++;
			while (true) {
				next = nextToken();
				if (!next.has_value())
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDefinition);
				if (next.value().GetType() == TokenType::COMMA) {
					auto err = analyseParameterDeclaration();
					if (err.has_value())
						return err;
					slot++;
				}
				else if (next.value().GetType() == TokenType::RIGHT_BRACKET) {
					unreadToken();
					break;
				}
				else {
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidFunctionDefinition);
				}
			}
		}
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoRightBracket);

		int n = _fun.size();
		_fun[n - 1]._params_size = slot;
		_funInstruction.emplace_back(functionBodyTable());
	}
	*/


	std::optional<CompilationError> Analyser::analyseParamDeclaration() {
		bool isConst = false;
		auto next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedTypeSpecifier);
		if (next.value().GetType() == TokenType::CONST)
			isConst = true;
		else
			unreadToken();
		next = nextToken();

		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidParam);

		if (!isTypeSpecifier(next.value()))
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedTypeSpecifier);
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedIdentifier);

		if(isConst)
			addConstant(next.value(), 1);
		else
			addVariable(next.value(), 1);

		return {};
	}
	
	std::optional<CompilationError> Analyser::analyseExpression() { 
		auto err = analyseMultiExpression();
		if (err.has_value())
			return err;
		while (true) {
			auto next = nextToken();
			if (!next.has_value() || !isPlusOrMinus(next.value())) {
				unreadToken();
				return {};
			}
			
			err = analyseMultiExpression();
			if (err.has_value())
				return err;
			
			if (_instructionIndex == -1) {
				if (next.value().GetType() == TokenType::PLUS_SIGN)
					_start.emplace_back(Operation::IADD, 0, 0);
				else if (next.value().GetType() == TokenType::MINUS_SIGN)
					_start.emplace_back(Operation::ISUB, 0, 0);
			}
			else {
				if (next.value().GetType() == TokenType::PLUS_SIGN)
					_funcInstructions[_instructionIndex]._funins.emplace_back(IADD, 0, 0);
				else if (next.value().GetType() == TokenType::MINUS_SIGN)
					_funcInstructions[_instructionIndex]._funins.emplace_back(ISUB, 0, 0);
			}
		}
		return {  };
	}

	//<multiplicative-expression> ::= 
	//<cast - expression>{<multiplicative - operator><cast - expression>}
	std::optional<CompilationError> Analyser::analyseMultiExpression() {
		auto err = analyseUnaryExpression();
		if (err.has_value())
			return err;
		while (true) {
			auto next = nextToken();
			if (!next.has_value() || !isMultiOrDevide(next.value())) {
				unreadToken();
				return {};
			}
			
			err = analyseUnaryExpression();
			if (err.has_value())
				return err;

			if (_instructionIndex == -1) {
				if (next.value().GetType() == TokenType::MULTIPLICATION_SIGN)
					_start.emplace_back(Operation::IMUL, 0, 0);
				else if (next.value().GetType() == TokenType::DIVISION_SIGN)
					_start.emplace_back(Operation::IDIV, 0, 0);
			}
			else {
				if (next.value().GetType() == TokenType::MULTIPLICATION_SIGN)
					_funcInstructions[_instructionIndex]._funins.emplace_back(IMUL, 0, 0);
				else if (next.value().GetType() == TokenType::DIVISION_SIGN)
					_funcInstructions[_instructionIndex]._funins.emplace_back(IDIV, 0, 0);
			}
		}
		return {};
	}
	/*
	// <cast-expression>
	std::optional<CompilationError> Analyser::analyseCastExpression() {
		while (true) {
			auto next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET) {
				unreadToken();
				break;
			}
			next = nextToken();
			if (!next.has_value() || !isTypeSpecifier(next.value())) {
				unreadToken();
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedTypeSpecifier);
			}
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET) {
				unreadToken();
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedRightBracket);
			}
			//
			//	类型转换
			//
		}
		auto err = analyseUnaryExpression();
		if (err.has_value())
			return err;
		return {};
	}
	*/

	// unary-expression
	std::optional<CompilationError> Analyser::analyseUnaryExpression() {
		auto next = nextToken();
		//unreadToken();
		int prefix = 1;
		if (next.has_value() && isPlusOrMinus(next.value())) {
			if (next.value().GetType() == TokenType::MINUS_SIGN)
				prefix = -1;
			next = nextToken();
		}
		if (!next.has_value()) {
			unreadToken();
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		}
		std::optional<CompilationError> err;
		std::optional<Token> preToken;
		std::string preTokenStr;
		int32_t x;
		switch(next.value().GetType()) {
		case TokenType::LEFT_BRACKET:
			err = analyseExpression();
			if (err.has_value())
				return err;
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedRightBracket);
			break;
		case TokenType::IDENTIFIER:
			preToken = next; 
			preTokenStr = preToken.value().GetValueString();
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET) {//<unary-expression>:=[<unary-operator>]<identifier>
				unreadToken();
				int n = _var.size();
				int addr = -1;
				int index = -1;
				for (int i = n - 1; i >= 0; i--) {
					if (preTokenStr == _var[i].getName()) {
						index = i;
						break;
					}
				}
				if (index == -1)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
				if (_var[index].getType() == 1)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotInitialized);
				if (_var[index].getType() == 3)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
				addr = _var[index].getAddress();
				int _offset = 0;
				int _level_diff = 0;
				if (_instructionIndex == -1) {
					_offset = addr;
					_level_diff = 0;
					_start.emplace_back(LOADA, _level_diff, _offset);
					_start.emplace_back(ILOAD, 0, 0);
				}
				else {
					_offset = addr - _indexTable[_var[index].getLevel()];
					_level_diff = 1 - _var[index].getLevel();
					
					_funcInstructions[_instructionIndex]._funins.emplace_back(LOADA, _level_diff, _offset);
					_funcInstructions[_instructionIndex]._funins.emplace_back(ILOAD, 0, 0);
				
				}
			}
			else {
				unreadToken();
				unreadToken();
				err = analyseFunctionCall();
				if (err.has_value())
					return err;
			}
			break;
		case TokenType::DECINTEGER:
		case TokenType::HEXINTEGER:
			x = atoi(next.value().GetValueString().c_str());
			if (_instructionIndex == -1)
				_start.emplace_back(IPUSH, x, 0);
			else
				_funcInstructions[_instructionIndex]._funins.emplace_back(IPUSH, x, 0);
			break;
		default:
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteExpression);
		}

		if (prefix == -1) {
			if (_instructionIndex == -1) {
				_start.emplace_back(Operation::INEG, 0, 0);
			}
			else {
				_funcInstructions[_instructionIndex]._funins.emplace_back(INEG, 0, 0);
			}
		}
		return {};
	}

	std::optional<CompilationError> Analyser::analyseCompoundStatement() {
		auto next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACE) {
			unreadToken();
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedLeftBrace);
		}
		auto err = analyseVariableDeclaration(false);
		if (err.has_value()) {
			return err;
		}
		err = analyseStatementseq();
		if (err.has_value())
			return err;
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACE) {
			unreadToken();
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedRightBrace);
		}
		return {};
	}

	std::optional<CompilationError> Analyser::analyseStatementseq() {
		while (true) {
			auto err = analyseStatement();
			if (err.has_value())
				return err;
			auto next = nextToken();
			unreadToken();
			if (next.value().GetType() == TokenType::RIGHT_BRACE)
				return {};
		}
	}

	std::optional<CompilationError> Analyser::analyseStatement() {
		auto next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteStatement);
		unreadToken();
		std::optional<CompilationError> err;
		std::string preTokenStr;
		switch (next.value().GetType())
		{
		case TokenType::LEFT_BRACE:
			err = analyseCompoundStatement();
			if (err.has_value())
				return err;
			break;
		case TokenType::RIGHT_BRACE:
			return {};
		case TokenType::IF:
			err = analyseConditionStatement();
			if (err.has_value())
				return err;
			break;
		case TokenType::WHILE:
			err = analyseLoopStatement();
			if (err.has_value())
				return err;
			break;
		case TokenType::PRINT:
			err = analysePrintStatement();
			if (err.has_value())
				return err;
			break;
		case TokenType::SCAN:
			err = analyseScanStatement();
			if (err.has_value())
				return err;
			break;
		case TokenType::RETURN:
			err = analyseJumpStatement();
			if (err.has_value())
				return err;
			break;
		case TokenType::IDENTIFIER: {
			preTokenStr = next.value().GetValueString();
			next = nextToken();
			next = nextToken();
			if (!next.has_value())
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteStatement);
			switch (next.value().GetType()) {
			case TokenType::EQUAL_SIGN: {
				int n = _var.size();
				int addr = -1;
				int index = -1;
				for (int i = n - 1; i >= 0; i--) {
					if (preTokenStr == _var[i].getName()) {
						index = i;
						break;
					}
				}
				if (index == -1)
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
				addr = _var[index].getAddress();
				if (_var[index]._type == 0) {
					return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToConstant);
				}
				int _offset = 0;
				int _level_diff = 0;
				_offset = addr - _indexTable[_var[index].getLevel()];
				_level_diff = 1 - _var[index].getLevel();
				_funcInstructions[_instructionIndex]._funins.emplace_back(LOADA, _level_diff, _offset);

				err = analyseExpression();
				if (err.has_value())
					return err;
				_funcInstructions[_instructionIndex]._funins.emplace_back(ISTORE, 0, 0);
				_var[index]._type = 2;
				return {};
			}
			case TokenType::LEFT_BRACKET: {
				unreadToken();
				unreadToken();
				err = analyseFunctionCall();
				if (err.has_value())
					return err;
				for (int i = 0; i < _fun.size(); i++) {
					if (_fun[i]._value == preTokenStr) {
						if (_fun[i]._haveReturnValue == 1) {
							_funcInstructions[_instructionIndex]._funins.emplace_back(POP, 0, 0);
						}
						break;
					}
				}
				return {};
			}
			default:
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidStatement);
			}
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
			break;
		}
		case TokenType::SEMICOLON:
			next = nextToken();
			break;
		default:
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidStatement);
		}
		return {};
	}

	std::optional<CompilationError> Analyser::analyseConditionStatement() {
		auto next = nextToken();
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET) {
			unreadToken();
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedLeftBracket);
		}
		auto err = analyseCondition();
		int index1 = _funcInstructions[_instructionIndex]._funins.size() - 1;
		if (err.has_value())
			return err;
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET) {
			unreadToken();
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedRightBracket);
		}
		err = analyseStatement();
		if (err.has_value())
			return err;

		_funcInstructions[_instructionIndex]._funins.emplace_back(JMP, 0, 0);
		int index2 = _funcInstructions[_instructionIndex]._funins.size() - 1;
		_funcInstructions[_instructionIndex]._funins[index1].SetX(index2 + 1);

		int off1 = _funcInstructions[_instructionIndex]._funins.size();
		_funcInstructions[_instructionIndex]._funins[index1].SetX(off1);

		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::ELSE) {
			unreadToken();
			return {};
		}
		else {
			err = analyseStatement();
			if (err.has_value())
				return err;
			int off2 = _funcInstructions[_instructionIndex]._funins.size();
			_funcInstructions[_instructionIndex]._funins[index2].SetX(off2);
		}
		return {};
	}

	std::optional<CompilationError> Analyser::analyseCondition() {
		auto err = analyseExpression();
		if (err.has_value())
			return err;
		auto next = nextToken();

		switch (next.value().GetType()) {
		case RIGHT_BRACKET: {
			_funcInstructions[_instructionIndex]._funins.emplace_back(JE, 0, 0);
			unreadToken();
			break;
		}
		case LESSTHAN_SIGN: {
			err = analyseExpression();
			if (err.has_value())
				return err;
			_funcInstructions[_instructionIndex]._funins.emplace_back(ISUB, 0, 0);
			_funcInstructions[_instructionIndex]._funins.emplace_back(JGE, 0, 0);
			break;
		}
		case LESSTHANOREQUAL_SIGN: {
			err = analyseExpression();
			if (err.has_value())
				return err;
			_funcInstructions[_instructionIndex]._funins.emplace_back(ISUB, 0, 0);
			_funcInstructions[_instructionIndex]._funins.emplace_back(JG, 0, 0);
			break;
		}
		case GREATERTHAN_SIGN: {
			err = analyseExpression();
			if (err.has_value())
				return err;
			_funcInstructions[_instructionIndex]._funins.emplace_back(ISUB, 0, 0);
			_funcInstructions[_instructionIndex]._funins.emplace_back(JLE, 0, 0);
			break;
		}
		case GREATERTHANOREQUAL_SIGN: {
			err = analyseExpression();
			if (err.has_value())
				return err;
			_funcInstructions[_instructionIndex]._funins.emplace_back(ISUB, 0, 0);
			_funcInstructions[_instructionIndex]._funins.emplace_back(JL, 0, 0);
			break;
		}
		case NOTEQUAL_SIGN: {
			err = analyseExpression();
			if (err.has_value())
				return err;
			_funcInstructions[_instructionIndex]._funins.emplace_back(ISUB, 0, 0);
			_funcInstructions[_instructionIndex]._funins.emplace_back(JE, 0, 0);
			break;
		}
		case DOUBLE_EQUAL: {
			err = analyseExpression();
			if (err.has_value())
				return err;
			_funcInstructions[_instructionIndex]._funins.emplace_back(ISUB, 0, 0);
			_funcInstructions[_instructionIndex]._funins.emplace_back(JNE, 0, 0);
			break;
		}
		default:
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteCondition);
		}

		return {};

	}

	std::optional<CompilationError> Analyser::analyseLoopStatement() {
		int off1 = _funcInstructions[_instructionIndex]._funins.size();
		auto next = nextToken();
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET) {
			unreadToken();
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedLeftBracket);
		}
		auto err = analyseCondition();
		if (err.has_value())
			return err;
		int index1 = _funcInstructions[_instructionIndex]._funins.size() - 1;
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET) {
			unreadToken();
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedRightBracket);
		}
		err = analyseStatement();
		if (err.has_value())
			return err;

		_funcInstructions[_instructionIndex]._funins.emplace_back(JMP, off1, 0);
		int off2 = _funcInstructions[_instructionIndex]._funins.size();
		_funcInstructions[_instructionIndex]._funins[index1].SetX(off2);

		return {};

	}

	std::optional<CompilationError> Analyser::analyseFunctionCall() {
		int params = 0;
		auto next = nextToken();
		auto tmp = next.value();

		int nf = _fun.size();
		bool haveFunction = false;
		for (int i = 0; i < nf; i++) {
			if (tmp.GetValueString() == _fun[i]._value)
				haveFunction = true;
		}

		if (!haveFunction)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteFunctionCall);

		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteFunctionCall);
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET) {
			unreadToken();
			auto err = analyseExpression();
			if (err.has_value())
				return err;
			params++;
			while (true) {
				next = nextToken();
				if (!next.has_value() || next.value().GetType() != TokenType::COMMA_SIGN) {
					unreadToken();
					break;
				}
				err = analyseExpression();
				if (err.has_value())
					return err;
				params++;
			}
		}
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteFunctionCall);

		nf = _fun.size();
		int tmpIndex = -1;
		for (int i = 0; i < nf; i++) {
			if (tmp.GetValueString() == _fun[i]._value)
				tmpIndex = i;
		}

		if (params == _fun[tmpIndex]._params_size)
			_funcInstructions[_instructionIndex]._funins.emplace_back(CALL, tmpIndex, 0);
		else
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteFunctionCall);
		return {};
	}
	/*
	std::optional<CompilationError> Analyser::analyseAssignmentStatement() {
		auto next = nextToken();
		auto tmp = next.value();
		if (!isDeclared(tmp.GetValueString()))
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
		if (isConstant(tmp.GetValueString()))
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrAssignToConstant);
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::EQUAL_SIGN) {
			unreadToken();
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidAssignment);
		}
		auto err = analyseExpression();
		if (err.has_value())
			return err;
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON) {
			unreadToken();
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
		}

		// 赋值命令
		_instructions.emplace_back(Operation::STO, getIndex(tmp.GetValueString()));
		

		if (isUninitializedVariable(tmp.GetValueString())) {
			// 删除
			updateUninitializedVar(tmp.GetValueString());
		}
		return {};
	}
	*/

	std::optional<CompilationError> Analyser::analysePrintStatement() {
		auto next = nextToken();
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET) {
			unreadToken();
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);
		}

		next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteStatement);
		else if (next.value().GetType() == TokenType::RIGHT_BRACKET) {
			_funcInstructions[_instructionIndex]._funins.emplace_back(PRINTL, 0, 0);
			next = nextToken();
			if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
			return {};
		}
		else {
			unreadToken();
		}

		auto err = analyseExpression();
		if (err.has_value())
			return err;

		_funcInstructions[_instructionIndex]._funins.emplace_back(IPRINT, 0, 0);

		while (true) {
			next = nextToken();
			if (!next.has_value())
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteStatement);
			if (next.value().GetType() == TokenType::RIGHT_BRACKET) {
				unreadToken();
				break;
			}
			if (next.value().GetType() != TokenType::COMMA_SIGN)
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteStatement);
			auto err = analyseExpression();
			if (err.has_value())
				return err;

			_funcInstructions[_instructionIndex]._funins.emplace_back(BIPUSH, 32, 0);
			_funcInstructions[_instructionIndex]._funins.emplace_back(CPRINT, 0, 0);
			_funcInstructions[_instructionIndex]._funins.emplace_back(IPRINT, 0, 0);
		}
		_funcInstructions[_instructionIndex]._funins.emplace_back(PRINTL, 0, 0);


		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET) {
			unreadToken();
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);
		}
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON) {
			unreadToken();
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
		}

		return {};
	}


	std::optional<CompilationError> Analyser::analyseScanStatement() {
		auto next = nextToken();
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::LEFT_BRACKET) {
			unreadToken();
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidScan);
		}
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::IDENTIFIER) {
			unreadToken();
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidScan);
		}

		auto preTokenStr = next.value().GetValueString();
		int n = _var.size();
		int addr = -1;
		int index = -1;
		for (int i = n - 1; i >= 0; i--) {
			if (preTokenStr == _var[i].getName()) {
				index = i;
				break;
			}
		}
		if (index == -1)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotDeclared);
		if (_var[index].getType() == 1)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNotInitialized);

		addr = _var[index].getAddress();
		int _offset = addr - _indexTable[_var[index].getLevel()];;
		int _level_diff = 1 - _var[index].getLevel();

		_funcInstructions[_instructionIndex]._funins.emplace_back(LOADA, _level_diff, _offset);
		_funcInstructions[_instructionIndex]._funins.emplace_back(ISCAN, 0, 0);
		_funcInstructions[_instructionIndex]._funins.emplace_back(ISTORE, 0, 0);

		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::RIGHT_BRACKET) {
			unreadToken();
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrInvalidPrint);
		}
		next = nextToken();
		if (!next.has_value() || next.value().GetType() != TokenType::SEMICOLON) {
			unreadToken();
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
		}
		// 生成相应的指令 SCAN
		//_instructions.emplace_back(Operation::WRT, 0);
		return {};
	}

	std::optional<CompilationError> Analyser::analyseJumpStatement() {
		auto next = nextToken();
		next = nextToken();
		if (!next.has_value())
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrIncompleteStatement);
		if (next.value().GetType() == TokenType::SEMICOLON) {
			if (_fun[_instructionIndex]._haveReturnValue == 1) {
				return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNeedReturnValue);
			}
			else {
				_funcInstructions[_instructionIndex]._funins.emplace_back(RET, 0, 0);
				return {};
			}
		}
		else
			unreadToken();
		auto err = analyseExpression();
		if (err.has_value())
			return err;
		if (_fun[_instructionIndex]._haveReturnValue == 0) 
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoNeedReturnValue);
		
		_funcInstructions[_instructionIndex]._funins.emplace_back(IRET, 0, 0);

		next = nextToken();
		if (next.value().GetType() != TokenType::SEMICOLON)
			return std::make_optional<CompilationError>(_current_pos, ErrorCode::ErrNoSemicolon);
		return {};
	}

	std::optional<Token> Analyser::nextToken() {
		if (_offset == _tokens.size())
			return {};
		// 考虑到 _tokens[0..._offset-1] 已经被分析过了
		// 所以我们选择 _tokens[0..._offset-1] 的 EndPos 作为当前位置
		_current_pos = _tokens[_offset].GetEndPos();
		return _tokens[_offset++];
	}

	void Analyser::unreadToken() {
		if (_offset == 0)
			DieAndPrint("analyser unreads token from the begining.");
		_current_pos = _tokens[_offset - 1].GetEndPos();
		_offset--;
	}

	void Analyser::_add(const Token& tk, int32_t type, int32_t level) {
		if (tk.GetType() != TokenType::IDENTIFIER)
			DieAndPrint("only identifier can be added to the table.");
		if (type == 3) {
			functionsTable f("S", 0, 1, tk.GetValueString());
			_fun.emplace_back(f);
			_instructionIndex++;
		}
		else {
			varTable v(tk.GetValueString(), type, level, _nextVarAddress);
			_nextVarAddress++;
			_var.emplace_back(v);
		}
		_nextTokenIndex++;
	}

	void Analyser::addConstant(const Token& tk, int32_t level) {
		_add(tk, 0, level);
	}

	void Analyser::addUninitializedVariable(const Token& tk, int32_t level) {
		_add(tk, 1, level);
	}

	void Analyser::addVariable(const Token& tk, int32_t level) {
		_add(tk, 2, level);
	}
	void Analyser::addFunction(const Token& tk, int32_t level) {
		_add(tk, 3, level);
	}
	
	int32_t Analyser::getIndex(const std::string& s, int32_t level) {
		int n = _var.size();
		for (int i = 0; i < n; i++) {
			if (s == _var[i].getName() && _var[i].getLevel() == level)
				return i;
		}
		return -1;
	}
	bool Analyser::isDeclared(const std::string& s, int32_t level) {//level为0则是全局的
		int n = _var.size();
		for (int i = 0; i < n; i++) {
			if (s == _var[i].getName() && _var[i].getLevel() == level)//各个变量的类型,0为常量,1为未赋值变量，2为已赋值变量,3为函数
				return true;
		}
		if (isFunctionName(s)) {
			return true;
		}
		return false;
	}
	bool Analyser::isConstant(const std::string& s, int32_t level) {
		int n = _var.size();
		for (int i = 0; i < n; i++) {
			if (s == _var[i].getName() && _var[i].getLevel() == level && _var[i].getType() == 0)
				return true;
		}
		return false;
	}
	bool Analyser::isUninitializedVariable(const std::string& s, int32_t level) {
		int n = _var.size();
		for (int i = 0; i < n; i++) {
			if (s == _var[i].getName() && _var[i].getLevel() == level && _var[i].getType() == 1)
				return true;
		}
		return false;
	}
	bool Analyser::isInitializedVariable(const std::string& s, int32_t level) {
		int n = _var.size();
		for (int i = 0; i < n; i++) {
			if (s == _var[i].getName() && _var[i].getLevel() == level && _var[i].getType() == 2)
				return true;
		}
		return false;
	}
	bool Analyser::isFunctionName(const std::string& s) {
		int n = _fun.size();
		for (int i = 0; i < n; i++) {
			if (s == _fun[i]._value && _instructionIndex == i)
				return true;
		}
		return false;
	}

	std::vector<Instruction> Analyser::getStartCode() {
		return _start;
	}
	std::vector<varTable> Analyser::getVarTable() {
		return _var;
	}
	std::vector<functionsTable> Analyser::getFunctionTable() {
		return _fun;
	}
	bool Analyser::isTypeSpecifier(const Token& tk) {
		auto tmp = tk.GetType();
		return (tmp == TokenType::INT || tmp == TokenType::VOID || tmp == TokenType::CHAR || tmp == TokenType::DOUBLE);
	}

	bool Analyser::isPlusOrMinus(const Token& tk) {
		auto tmp = tk.GetType();
		return (tmp == TokenType::PLUS_SIGN || tmp == TokenType::MINUS_SIGN);
	}

	bool Analyser::isMultiOrDevide(const Token& tk) {
		auto tmp = tk.GetType();
		return (tmp == TokenType::MULTIPLICATION_SIGN || tmp == TokenType::DIVISION_SIGN);
	}

	bool Analyser::isRelationalOperator(const Token& tk) {
		auto tmp = tk.GetType();
		return (tmp == TokenType::LESSTHAN_SIGN || tmp == TokenType::LESSTHANOREQUAL_SIGN || tmp == TokenType::GREATERTHAN_SIGN
			|| tmp == TokenType::GREATERTHANOREQUAL_SIGN || tmp == TokenType::DOUBLE_EQUAL || tmp == TokenType::NOTEQUAL_SIGN);
	}

	int32_t Analyser::isOverflow(const std::string&s)
	{
		int64_t res = 0;
		for (int i = 0; i < s.size(); i++)
		{
			res *= 10;
			res += (s[i] - (int64_t)'0');
		}
		if (res > 2147483647 || res < 0)
			return -1;
		return res;
	}
}