#pragma once

#include <cstdint>
#include <utility>

namespace plc0 {

	enum Operation {
		NOP = 0,
		IPUSH = 0x02,
		BIPUSH,
		POP = 0x04,
		POP2 = 0x05,
		POPN = 0x06,
		DUP = 0x07,
		LOADC = 0x09,
		LOADA = 0x0a,
		NEW = 0x0b,
		SNEW = 0x0c,
		ILOAD = 0x10,
		ALOAD = 0x12,
		IALOAD = 0x18,
		ISTORE = 0x20,
		IASTORE = 0x28,
		IADD = 0x30,
		ISUB = 0x34,
		IMUL = 0x38,
		IDIV = 0x3c,
		INEG = 0x40,
		ICMP = 0x44,
		JMP = 0x70,
		JE = 0x71,
		JNE = 0x72,
		JL = 0x73,
		JGE = 0x74,
		JG = 0x75,
		JLE = 0x76,
		CALL = 0x80,
		RET = 0x88,
		IRET = 0x89,
		IPRINT = 0xa0,
		CPRINT,
		PRINTL = 0xaf,
		ISCAN = 0xb0,
	};
	
	class Instruction final {
	private:
		using int32_t = std::int32_t;
	public:
		friend void swap(Instruction& lhs, Instruction& rhs);
	public:
		Instruction() : Instruction(Operation::NOP, 0) {}
		Instruction(Operation opr) : _opr(opr) {}
		Instruction(Operation opr, int32_t x) : _opr(opr), _x(x) {}
		Instruction(Operation opr, int32_t x, int32_t y) : _opr(opr), _x(x), _y(y) {}
		Instruction(const Instruction& i) { _opr = i._opr; _x = i._x; _y = i._y; }
		Instruction(Instruction&& i) :Instruction() { swap(*this, i); }

		Instruction& operator=(Instruction i) { swap(*this, i); return *this; }
		bool operator==(const Instruction& i) const { return _opr == i._opr && _x == i._x && _y == i._y; }
		void SetX(int32_t x) { _x = x; }
		void SetY(int32_t y) { _y = y; }

		Operation GetOperation() const { return _opr; }
		int32_t GetX() const { return _x; }
		int32_t GetY() const { return _y; }
	private:
		Operation _opr;
		int32_t _x;
		int32_t _y;
	};

	inline void swap(Instruction& lhs, Instruction& rhs) {
		using std::swap;
		swap(lhs._opr, rhs._opr);
		swap(lhs._x, rhs._x);
	}
}