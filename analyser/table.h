#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include "instruction/instruction.h"

namespace plc0 {
	using int32_t = std::int32_t;

	// ���ű�
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
		int32_t _type;      //0Ϊ����, 1Ϊδ��ֵ������2Ϊ�Ѹ�ֵ����, 3Ϊ����
		int32_t _level;     //0Ϊȫ�֣�1Ϊ�ֲ�
		int32_t _address;   //��ջ�е�λ��
	};
	

	// ������/������
	class functionsTable {
	public:
		functionsTable(std::string type, int32_t params_size, int32_t level, std::string value) :
			_type(type), _params_size(params_size), _level(level), _value(value), _haveReturnValue(-1) {}
	public:
		//        int32_t name_index;     // ��������.constants�е��±�
		std::string _type;
		int32_t _params_size;    //����ռ�õ�slot��
		int32_t _level;          //����Ƕ�׵Ĳ㼶
		std::string _value;
		int32_t _haveReturnValue;
	};

	class functionInstructions {
	public:
		std::vector<Instruction> _funins;
	};

}