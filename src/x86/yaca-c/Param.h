#ifndef _PARAM_H_
#define _PARAM_H_

#include <string>

using namespace std;

class Param {
private:
	string m_type;
	string m_name;
	int m_size;
	int m_reg;

public:
	Param(): m_size(0), m_reg(0) {}
	Param(string type, string name, int size, int reg): m_type(type), m_name(name), m_size(size), m_reg(reg) {}
	void init(string type, string name, int size, int reg) {
		m_type = type;
		m_name = name;
		m_size = size;
		m_reg = reg;
	}

	string getType() { return m_type; }
	string getName() { return m_name; }
	int getSize() { return m_size; }
	int getReg() { return m_reg; }
};

#endif /* _PARAM_H_ */
