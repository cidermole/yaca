#ifndef _EXPORT_H_
#define _EXPORT_H_

#include <iostream>
#include <list>
#include "Param.h"
#include "XmlTree.h"

using namespace std;

class Message;
class Source;

enum mtype { MSG, RTR };

class Message {
private:
	string m_name;
	string m_nameSpace;
	list<Param> m_params;
	int m_size;
	int m_regPlace;
	mtype m_type;
	unsigned char m_packStyle;
	unsigned char m_packIndex;

	static void _createInlineAsm(stringstream& ss, Param param);
	static XmlTree *makeEepromDescriptorNode();

public:
	Message(): m_size(0), m_regPlace(26), m_packStyle(0), m_packIndex(0) {}
	Message(string name): m_name(name), m_size(0), m_regPlace(26), m_packStyle(0), m_packIndex(0) {}
	int getSize() { return m_size; }
	void addParam(string type, string name); // throws const char*
	void addParam(string type); // throws const char*
	string getName() { return m_name; }
	list<Param> getParams() { return m_params; }
//	int getParamCount() { return m_params.size(); }
	void setName(string name) { m_name = name; }
	void setNameSpace(string nameSpace) { m_nameSpace = nameSpace; }
	string getNameSpace() { return m_nameSpace; }

	void setType(mtype type) { m_type = type; }
	mtype getType() { return m_type; }

	int getPackStyle() { return m_packStyle; }

	static string createRemoteHeader(Source* src);
	static string createSendCode(Source* src);
	static string createFunctionTable(Source* src);
	static XmlTree createXml(Source* src);
};

#endif /* _EXPORT_H_ */


/*

need to make a list of Export classes

Export:
export name
data types (list of int's containing data type width)


*/
