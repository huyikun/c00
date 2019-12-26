#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "instruction/instruction.h"

namespace plc0
{
using int32_t = std::int32_t;

class varTable
{
public:
	varTable(std::string name, int32_t type, int32_t level, int32_t address) : _name(name), _type(type), _level(level), _address(address) {}
	varTable() : varTable("", -1, -1, -1) {}

	std::string getName() { return _name; }
	int32_t getType() { return _type; }
	int32_t getLevel() { return _level; }
	int32_t getAddress() { return _address; }

	void setType(int32_t type) { _type = type; }
	void setAddress(int32_t address) { _address = address; }

	//private:
public:
	std::string _name;
	int32_t _type;
	int32_t _level;
	int32_t _address;
};

class functionsTable
{
public:
	functionsTable(std::string type, int32_t params_size, int32_t level, std::string value) : _type(type), _params_size(params_size), _level(level), _value(value), _haveReturnValue(-1) {}

public:
	std::string _type;
	int32_t _params_size;
	int32_t _level;
	std::string _value;
	int32_t _haveReturnValue;
};

class functionInstructions
{
public:
	std::vector<Instruction> _funins;
};

} // namespace plc0