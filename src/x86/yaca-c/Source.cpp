#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <windows.h>
#else
#include <unistd.h>
#endif
#include "Source.h"
#include "Globals.h"
#include "Utils.h"
#include "Template.h"
#include "Message.h"

using namespace std;

map<string, int> Source::m_sizeofCache;

// TODO: reimplement this throwable() goop with a define ->  string() + ""
// (or string("")? was that some performance issue in the docs?

void Source::makeObjName() {
	size_t dotPos = m_file.find_last_of(".");

	if(dotPos != string::npos) {
		m_object = m_file.substr(0, dotPos) + ".o";
	} else if(Globals::getInt("verbose") >= 1) {
		cerr << "Warning: Why does source file \"" << m_file << "\" not have an extension?" << endl;
		m_object = m_file + ".o";
	}
}

void Source::recompile() {
	m_compiled = false;
	this->compile();
}

void Source::compile() {
	throwable();
	int gccCode;
	ifstream file;
	string line;
	stringstream gccError;

	if(m_compiled)
		return;

	if(Globals::getInt("verbose") >= 2)
		cout << "Info: running command \"" << "avr-gcc -o \"" << m_object << "\" \"" << m_file << "\" -c -O2 -mmcu=" << Globals::getStr("mcu") << " " << Globals::getStr("compilerOptions") << " 2>\"" << Globals::getStr("tmpfile") << "\"\"" << endl;
	gccCode = system((ES + "avr-gcc -o \"" + m_object + "\" \"" + m_file + "\" -c -O2 -mmcu=" + Globals::getStr("mcu") + " " + Globals::getStr("compilerOptions") + " 2>\"" + Globals::getStr("tmpfile") + "\"").c_str());
	if(gccCode) {
		file.open(Globals::getStr("tmpfile").c_str(), ifstream::in);
		if(!file.good()) {
			if(Globals::getInt("verbose") >= 2)
				cerr << "Warning: failed to open temporary file \"" << Globals::getStr("tmpfile") << "\"" << endl;
			throw_error("Compiling \"" + m_file + "\" went wrong, but could not open temporary error file.");
		}
		while(getline(file, line))
			gccError << endl << line;

		cleanup(Globals::getStr("tmpfile"));
		
		throw_error("Compiler error on \"" + m_file + "\". Passing through compiler output:" + gccError.str());
	}
	magic_unlink(Globals::getStr("tmpfile").c_str());
	m_compiled = true;
}

string Source::getSymbols() {
	throwable();
	ifstream file;
	string line;
	stringstream symbols;

	if(!m_compiled)
		compile();

	if(Globals::getInt("verbose") >= 2)
		cout << "Info: running command \"" << "avr-nm -C \"" << m_object << "\" > \"" << Globals::getStr("tmpfile") << "\"\"" << endl;
	system((ES + "avr-nm -C \"" + m_object + "\" > \"" + Globals::getStr("tmpfile") + "\"").c_str());
	file.open(Globals::getStr("tmpfile").c_str(), ifstream::in);
	if(!file.good()) {
		if(Globals::getInt("verbose") >= 2)
			cerr << "Warning: failed to open temporary file \"" << Globals::getStr("tmpfile") << "\"" << endl;
		throw_error("Could not get symbol list of object \"" + m_object + "\" - failed to open temporary file.");
	}
	while(getline(file, line)) {
		if(symbols.tellp())
			symbols << endl << line;
		else
			symbols << line;
	}
	cleanup(Globals::getStr("tmpfile"));
	return symbols.str();
}

int Source::sizeOf(string type) {
	throwable();
	Template tpl(Globals::getStr("sizeofTemplate"));
	ofstream file;
	ifstream errfile;
	string line;
	stringstream gccError;
	int gccCode, rv;

	if(m_sizeofCache[type]) {
		if(Globals::getInt("verbose") >= 3)
			cout << "Info: cache hit determining sizeof(" + type + ") - already resolved to " << m_sizeofCache[type] << endl;
		return m_sizeofCache[type];
	}

	file.open((Globals::getStr("tmpfile2") + ".C").c_str(), ofstream::out);

	if(!file.good())
		throw_error("Source::sizeOf(): Could not open temporary file \"" + Globals::getStr("tmpfile2") + ".C\" for writing");

	tpl.addKey("<TYPE>", type);
	file << tpl.parse();
	file.close();

	line = ES + "avr-gcc -o \"" + Globals::getStr("tmpfile2") + ".o\" \"" + Globals::getStr("tmpfile2") + ".C\" -c -O2 2>\"" + Globals::getStr("tmpfile") + "\"";
	if(Globals::getInt("verbose") >= 2)
		cout << "Info: running command \"" << line << "\"" << endl;
	gccCode = system(line.c_str());

	if(gccCode) {
		errfile.open(Globals::getStr("tmpfile").c_str(), ifstream::in);
		if(!errfile.good()) {
			if(Globals::getInt("verbose") >= 2)
				cerr << "Warning: failed to open temporary file \"" << Globals::getStr("tmpfile") << "\"" << endl;
			throw_error("While trying to determine sizeof(" + type + "): Compiling temporary file\"" + Globals::getStr("tmpfile2") + ".C\" went wrong, but could not open temporary error file.");
		}
		while(getline(errfile, line))
			gccError << endl << line;

		if(Globals::getInt("verbose") >= 2)
			cerr << "Warning: keeping temporary file \"" + Globals::getStr("tmpfile2") + ".C\" for examination." << endl;
		else
			cleanup(Globals::getStr("tmpfile2") + ".C");

		throw_error("Compiler error while trying to determine sizeof(" + type + "). Passing through compiler output:" + gccError.str());
	}

	line = "avr-objdump -S \"" + Globals::getStr("tmpfile2") + ".o\" > \"" + Globals::getStr("tmpfile") + "\"";
	if(Globals::getInt("verbose") >= 2)
		cout << "Info: running command \"" << line << "\"" << endl;
	system(line.c_str());

	rv = _sizeOf(Globals::getStr("tmpfile").c_str());

	cleanup(Globals::getStr("tmpfile"));
	cleanup(Globals::getStr("tmpfile2") + ".C");
	cleanup(Globals::getStr("tmpfile2") + ".o");

	if(!rv)
		throw_error("Could not parse output of avr-objdump -S somehow (mhm, interesting)...");

	if(Globals::getInt("verbose") >= 2)
		cout << "Info: sizeof(" + type + ") = " << rv << endl;

	m_sizeofCache[type] = rv;
	return rv;
}

int Source::_sizeOf(const char* file) {
	throwable();
	FILE* f = fopen(file, "r");
	char c;
	char buf[3];

	if(!f)
		throw_error("Could not open temporary file containing the output of avr-objdump -S");

	c = fgetc(f);
	while(!feof(f) && c != ';')
		c = fgetc(f);
	if(c == ';') {
		fgetc(f);
		buf[0] = fgetc(f);
		buf[1] = fgetc(f);
		if(buf[1] >= '0' && buf[1] <= '9')
			buf[2] = '\0';
		else
			buf[1] = '\0';

		fclose(f);
		return atoi(buf);
	}
	fclose(f);
	throw_error("EOF parsing output of avr-objdump -S");
}

Message Source::parseExportLine(string line, bool omitFirst) {
	throwable();
	Message exp;
	size_t openBracket = line.find("("), closeBracket = line.find(")"), comma, fn;
	string paramList;
	int i;

	if(openBracket == string::npos)
		throw_error("Parse error in symbol list created by avr-nm: no opening bracket found");
	if(closeBracket == string::npos)
		throw_error("Parse error in symbol list created by avr-nm: no closing bracket found");

	paramList = line.substr(openBracket + 1, closeBracket - openBracket - 1);
	fn = line.find("::__msg_");
	if(fn == string::npos) {
		fn = line.find("::__rtr_");
		if(fn == string::npos)
			throw_error("Parse error in symbol list: no message prefix found");
		exp.setType(RTR);
	} else {
		exp.setType(MSG);
	}
	exp.setName(line.substr(fn + 8, openBracket - fn - 8));
	exp.setNameSpace(line.substr(NM_SNAME_POS, fn - NM_SNAME_POS));
//	exp.addParam("unsigned long", "canid"); // implicit CAN-ID parameter

	for(i = 0; paramList.length(); i++) {
		comma = paramList.find(", ");
		if(!omitFirst || i)
			exp.addParam(paramList.substr(0, comma == string::npos ? paramList.length() : comma));
		if(comma != string::npos)
			paramList = paramList.substr(comma + 2);
		else
			paramList = "";
	}

	return exp;
}

bool Source::checkExport(string line) {
	throwable();
	string nameStart = line.substr(NM_SNAME_POS);

	if(nameStart.length() < 2)
		throw_error("Parse error in symbol list");

	if(line.substr(NM_STYPE_POS, 1) != "T")
		return false;

	if(nameStart[0] != EXTERN_PREFIX || nameStart[1] < 'A' || nameStart[1] > 'Z')
		return ((line.find("__msg_") != string::npos) || (line.find("__rtr_") != string::npos));
	else
		return false;
}

bool Source::checkExportForR(string line) {
	throwable();
	string nameStart = line.substr(NM_SNAME_POS);

	if(nameStart.length() < 2)
		throw_error("Parse error in symbol list");

	if(line.substr(NM_STYPE_POS, 1) != "T" && line.substr(NM_STYPE_POS, 1) != "W")
		return false;

	if(nameStart[0] != EXTERN_PREFIX || nameStart[1] < 'A' || nameStart[1] > 'Z')
		return ((line.find("__msg_") != string::npos) || (line.find("__rtr_") != string::npos));
	else
		return false;
}

bool Source::checkSend(string line) {
	throwable();
	string nameStart = line.substr(NM_SNAME_POS);

	if(nameStart.length() < 2)
		throw_error("Parse error in symbol list");

	if(line.substr(NM_STYPE_POS, 1) != "U")
		return false;

	if(nameStart[0] == EXTERN_PREFIX && nameStart[1] >= 'A' && nameStart[1] <= 'Z')
		return ((line.find("__msg_") != string::npos) || (line.find("__rtr_") != string::npos));
	else
		return false;
}

list<Message> Source::getSends() {
	return _getMessages(checkSend);
}

list<Message> Source::getExports() {
	return _getMessages(checkExport);
}

list<Message> Source::getExportsForR() {
	return _getMessages(checkExportForR);
}

list<Message> Source::_getMessages(bool (*checkFct)(string)) {
	string symbols = getSymbols();
	string line;
	stringstream ss(symbols, stringstream::in);
	list<Message> l;
	Message e;

	if(Globals::getInt("verbose") >= 3)
		cout << "Info: processing symbol table:" << endl << symbols << endl << endl;

	while(getline(ss, line)) {
		if(checkFct(line)) {
			e = parseExportLine(line, /*checkSend == checkFct*/ false); // FIXME: check if this bricks something else too?
			l.push_back(e);
		}
	}

	return l;
}

void Source::cleanup(string file) {
	if(magic_unlink(file.c_str())) {
		if(Globals::getInt("verbose") >= 1)
			cerr << "Warning: Source::cleanup(): Could not remove temporary file \"" << file << "\"" << endl;
	}
}

