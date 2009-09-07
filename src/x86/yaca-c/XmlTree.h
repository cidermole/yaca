#ifndef XMLTREE_H
#define XMLTREE_H

#include <iostream>
#include <string>
#include <map>
#include <list>

#define START_BUF_SIZE 1024

#define gt_assert(a, code, desc, mark) \
	do { \
		if(!(a)) { \
			m_err = XmlError(code, (char *)(desc)); \
			goto mark; \
		} \
	} while(0)

#define th_assert(a, code, desc) \
	do { \
		if(!(a)) throw XmlError(code, (char *)(desc)); \
	} while(0)

// error codes: <   100: general errors
// error codes: >= 1000: parse errors

// last digit of error code gives the exact location in the code, while the rest gives a hint about the general type of the error

#define WHITESPACE " \n\r\t"

struct XmlError {
public:
	XmlError(): code(0), desc((char *)"no error") {}
	XmlError(int _code, char *_desc): code(_code), desc(_desc) {}
	
	int code;
	char *desc;
};

class XmlTree {
public:
	XmlTree(std::string name = "", std::string text = ""): m_name(name), m_text(text) {}
	~XmlTree();
	
	void read(const char *filename);
	void write(const char *filename);
	
	std::list<XmlTree *>::iterator begin() { return m_children.begin(); }
	std::list<XmlTree *>::iterator end() { return m_children.end(); }
	void append(XmlTree *child) { m_children.push_back(child); }
	
	std::string name() { return m_name; }
	std::string text() { return m_text; }
	std::string attribute(std::string attr) { return m_attrs[attr]; }
	
	void setName(std::string name) { m_name = name; }
	void setText(std::string text) { m_text = text; }
	void setAttribute(std::string attr, std::string val) { m_attrs[attr] = val; }

private:
	XmlError m_err;
	std::string m_name;
	std::string m_text;
	std::map<std::string, std::string> m_attrs;
	std::list<XmlTree *> m_children;
	
	std::string _trim(std::string s);
	std::string _attr_gen();
	void _write(std::ostream& ofs, std::string indent);
	void _process(std::list<std::pair<char *, int> >::iterator& it, std::list<std::pair<char *, int> >::iterator end, bool root);
	void _process_attributes(char *buf, int len);
	int _read(char **buffer, const char *filename);
	int _ff(char *buf, int len, char *str, int pos);
};

#endif /* XMLTREE_H */

