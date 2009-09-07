#include <iostream>
#include <fstream>
#include <cstdarg>
#include <sstream>
#include "Template.h"

string Template::parse() {
	throwable();
	ifstream file(m_file.c_str(), ifstream::in);
	string line;
	stringstream lines;
	string ls;
	map<string, string>::iterator mi;
	size_t pos;

	if(!file.good())
		throw_error("Could not open template file \"" + m_file + "\"");

	while(getline(file, line)) {
		if(lines.tellp())
			lines << endl << line;
		else
			lines << line;
	}
	ls = lines.str();

	for(mi = m_replace.begin(); mi != m_replace.end(); mi++) {
		pos = ls.find((*mi).first);
		while(pos != string::npos) {
			ls.replace(pos, (*mi).first.length(), (*mi).second);
			pos = ls.find((*mi).first);
		}
	}
	return ls;
}
