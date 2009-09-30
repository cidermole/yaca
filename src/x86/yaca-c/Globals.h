#ifndef _GLOBALS_H_
#define _GLOBALS_H_

#include <string>
#include <map>

using namespace std;

enum type_t { STR, INT };

class IntelliPtr {
private:
	void* m_ptr;
	type_t m_type;
public:
	IntelliPtr(void* ptr, type_t type): m_ptr(ptr), m_type(type) {}
	IntelliPtr(): m_ptr(NULL) {}
	void destroy() {
		switch(m_type) {
		case STR:
			delete ((string*)m_ptr);
			break;
		case INT:
			delete ((int*)m_ptr);
			break;
		}
	}
	void* get() {
		return m_ptr;
	}
};

class Globals {
private:
	static map<string, IntelliPtr> m_vars;
public:
	static string getStr(string name) {
		return string(*((string*)(m_vars[name].get())));
	}

	static void setStr(string name, string value) {
		m_vars[name] = IntelliPtr(new string(value), STR);
	}

	static int getInt(string name) {
		return *((int*)(m_vars[name].get()));
	}

	static void setInt(string name, int value) {
		m_vars[name] = IntelliPtr(new int, INT);
		*((int*)(m_vars[name].get())) = value;
	}

	~Globals();
};

#endif /* _GLOBALS_H_ */
