#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "instruction/instruction.h"

namespace plc0 {
	using int32_t = std::int32_t;

	// 符号表
	class varTable {
	public:
		varTable(std::string name, int32_t type, int32_t level, int32_t address) :
			_name(name), _type(type), _level(level), _address(address) {}
		varTable() :varTable("", -1, -1, -1) {}

		std::string getName() { return _name; }
		int32_t getType() { return _type; }
		int32_t getLevel() { return _level; }
		int32_t getAddress() { return _address; }

		void setType(int32_t type) { _type = type; }
		void setAddress(int32_t address) { _address = address; }

	//private:
	public:
		std::string _name;
		int32_t _type;      //0为常量, 1为未赋值变量，2为已赋值变量, 3为函数
		int32_t _level;     //0为全局，1为局部
		int32_t _address;   //在栈中的位置
	};
	

	// 函数表/常量表
	class functionsTable {
	public:
		functionsTable(std::string type, int32_t params_size, int32_t level, std::string value) :
			_type(type), _params_size(params_size), _level(level), _value(value), _haveReturnValue(-1) {}
	public:
		//        int32_t name_index;     // 函数名在.constants中的下标
		std::string _type;
		int32_t _params_size;    //参数占用的slot数
		int32_t _level;          //函数嵌套的层级
		std::string _value;
		int32_t _haveReturnValue;
	};

	class functionInstructions {
	public:
		std::vector<Instruction> _funins;
	};

}