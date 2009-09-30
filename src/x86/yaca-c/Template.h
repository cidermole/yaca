#ifndef _TEMPLATE_H_
#define _TEMPLATE_H_

#include <map>
#include <string>
#include "Utils.h"

using namespace std;

class Template {
private:
	string m_file;
	map<string, string> m_replace;

public:
	Template(string file): m_file(file) {}

	void addKey(string key, string value) {
		m_replace.insert(pair<string, string>(key, value));
	}

	string parse(); // throws const char*
};

#endif /* _TEMPLATE_H_ */
