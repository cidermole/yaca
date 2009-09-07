#ifndef _SOURCE_H_
#define _SOURCE_H_

#include <string>
#include <map>
#include <list>

#ifdef _MSC_VER
#define magic_unlink(a) _unlink(a)
#else
#define magic_unlink(a) unlink(a)
#endif

#define NM_SNAME_POS 11
#define NM_STYPE_POS 9
#define EXTERN_PREFIX 'R'

using namespace std;

class Message;

class Source {
private:
	string m_file;
	string m_object;
	bool m_compiled;
	static map<string, int> m_sizeofCache;

	static void cleanup(string file);
	void makeObjName();
	static int _sizeOf(const char* file);
	Message parseExportLine(string line, bool omitFirst);
	static bool checkExport(string line);
	static bool checkExportForR(string line);
	static bool checkSend(string line);
	list<Message> _getMessages(bool (*checkFct)(string)); // throws const char*

public:
	Source(): m_compiled(false) {}
	Source(string file): m_file(file), m_compiled(false) { makeObjName(); }
	void setFile(string file) {
		m_file = file;
		makeObjName();
	}
	void recompile(); // see below
	void compile(); // throws const char*
	string getSymbols(); // throws const char*
	list<Message> getExports(); // throws const char*
	list<Message> getExportsForR(); // throws const char*
	list<Message> getSends(); // throws const char*

	static int sizeOf(string type); // throws const char*
};

#endif /* _SOURCE_H_ */
