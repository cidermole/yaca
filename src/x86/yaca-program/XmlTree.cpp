#include "XmlTree.h"
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <utility>

using namespace std;

XmlTree::~XmlTree() {
	list<XmlTree *>::iterator it;
	
	for(it = begin(); it != end(); it++)
		delete (*it);
}

void XmlTree::write(const char *filename) {
	ofstream ofs(filename);
	list<XmlTree *>::iterator it;
	
	th_assert(ofs.good(), 11, "can't open output file");
	
	_write(ofs, "");
	ofs.close();
}

void XmlTree::_write(ostream& ofs, string indent) {
	list<XmlTree *>::iterator it;
	string s;
	
	if(m_name != "")
		ofs << indent << "<" << m_name << _attr_gen() << ((m_children.size() || m_text.length()) ? ">" : "/>") << "\n";
	
	ofs << (m_text.length() ? s + "\t" + indent : s + "") << m_text << (m_text.length() ? "\n" : "");
	for(it = begin(); it != end(); it++)
		(*it)->_write(ofs, indent + (m_name.length() ? "\t" : ""));
	
	if(m_name != "" && (m_children.size() || m_text.length()))
		ofs << indent << "</" << m_name << ">\n";
}

string XmlTree::_attr_gen() {
	string s;
	map<string, string>::iterator mit;
	
	for(mit = m_attrs.begin(); mit != m_attrs.end(); mit++)
		s = s + " " + mit->first + "=\"" + mit->second + "\"";
	
	return s;
}

void XmlTree::read(const char *filename) {
	char *buf;
	list<pair<char *, int> > chunks;
	int i, len;
	bool in_tag = false;
	pair<char *, int> cur;
	
	len = _read(&buf, filename);
	
	/************** tokenize at '<', remove comments and special tags **************/
	
	for(i = 0; i < len; i++) {
		if(buf[i] == '<' && i < len - 1) {
			if(in_tag) {
				cur.second = (int) (&buf[i] - cur.first);
				chunks.push_back(cur);
				in_tag = false;
			}
			
			if(buf[i + 1] == '!') {
				if(i < len - 3 && !strncmp(&buf[i], "<!--", strlen("<!--"))) {
					i = _ff(buf, len, (char *) "-->", i + 4);
					continue;
				} else {
					buf[i + 1] = '?';
				}
			}
			if(buf[i + 1] == '?') {
				i = _ff(buf, len, (char *) ">", i + 2);
			} else {
				in_tag = true;
				cur.first = &buf[i + 1];
			}
		}
	}
	if(in_tag) {
		cur.second = (int) (&buf[i] - cur.first);
		chunks.push_back(cur);
	}

	/************** parse the file **************/
	
	list<pair<char *, int> >::iterator ite = chunks.begin();
	
	_process(ite, chunks.end(), true);
}

bool my_strncmp(const char *s1, const char *s2) {
	if(s1 == NULL || s2 == NULL)
		return false;
	
	return !strncmp(s1, s2, strlen(s2));
}

void XmlTree::_process(list<pair<char *, int> >::iterator& it, list<pair<char *, int> >::iterator end, bool root) {
	char *tagname = NULL, ex;
	char *txt;
	int i, j;
	
	if(!root) {
		tagname = it->first;
		
		for(i = 0; i < it->second && tagname[i] != ' ' && tagname[i] != '/' && tagname[i] != '>'; i++);
		for(j = 0; j < it->second && tagname[j] != '>'; j++);
		th_assert(j < it->second, 1013, "tag is not properly closed with '>'");
		
		ex = it->first[i];
		it->first[i] = '\0';
		m_name = tagname;
		it->first[i] = ex;
		
		if(ex == ' ') {
			// parse attributes
			th_assert(i < it->second - 1, 1011, "tag is not properly closed with '>'");
			_process_attributes(&it->first[i + 1], it->second - i - 1);
		}
		// parse text
		if(j + 1 < it->second) {
			// XXX: overrun for +1 byte, need the buffer to catch this
			ex = it->first[it->second];
			it->first[it->second] = '\0';
			txt = &it->first[j + 1];
			m_text = txt;
			it->first[it->second] = ex;
			m_text = _trim(m_text);
		}
		
		if(it->first[j - 1] == '/') // tag is closing itself, no need for further processing
			return;
		it++;
	}
	
	for(; it != end && !my_strncmp(&it->first[1], tagname); it++) {
		th_assert(it->first[0] != '/', 1000, "closing tag before opening tag");
		XmlTree *child = new XmlTree();
		th_assert(child != NULL, 20, "out of memory");
		child->_process(it, end, false);
		m_children.push_back(child);
		if(it == end)
			break;
	}
	th_assert(my_strncmp(&it->first[1], tagname) || root, 1001, "no closing tag");
}

void XmlTree::_process_attributes(char *buf, int len) {
	int i = 0;
	char *attr, *val;
	string a, v;
	
	while(1) {
		for(; i < len && buf[i] == ' '; i++);
		th_assert(i < len, 1012, "tag is not properly closed with '>'");
	
		if(buf[i] == '>' || buf[i] == '/')
			return;
	
		attr = &buf[i];
		for(; i < len && buf[i] != '='; i++);
		th_assert(i < len - 2, 1020, "attribute name is not properly terminated with '=\"' or tag is not properly closed with '>'");
		th_assert(buf[i + 1] == '"', 1021, "attribute value is not enclosed in '\"'");
		buf[i] = '\0';
		
		val = &buf[i + 2];
		for(i += 2; i < len && buf[i] != '"'; i++);
		th_assert(i < len - 1, 1022, "attribute value is not enclosed in '\"' or tag is not properly closed with '>'");
		buf[i] = '\0';
		i++;
	
		a = attr;
		v = val;
		m_attrs[a] = v;
	}
}

// _ff: fast forward to a specified string, i.e. "-->"
// returns the advanced pos
int XmlTree::_ff(char *buf, int len, char *str, int pos) {
	int i, j;
	
	for(i = pos, j = 0; i < len && j < strlen(str); i++, j++) {
		if(buf[i] != str[j]) {
			if(buf[i] == str[0])
				j = 0;
			else
				j = -1;
		}
	}
	if(!strcmp(str, "-->"))
		th_assert(i < len, 30, "eof while searching for end of comment");
	else
		th_assert(i < len, 31, "eof while searching for end of tag");
	
	return (i - 1);
}

// read a file and return its buffer and size
int XmlTree::_read(char **buffer, const char *filename) {
	ifstream ifs(filename);
	char *buf = (char *) malloc(START_BUF_SIZE);
	streamsize used = 0, total = START_BUF_SIZE;
	
	gt_assert(ifs.good(), 10, "can't open input file", assert_failed);
	gt_assert(buf != NULL, 21, "out of memory", assert_failed);
	
	ifs.read(buf, total - used);
	while(ifs.good() && ifs.gcount() == total - used) {
		used += ifs.gcount();
		
		total *= 2;
		buf = (char *) realloc(buf, total);
		gt_assert(buf != NULL, 22, "out of memory", assert_failed);
		
		ifs.read(&buf[used], total - used);
	}
	// Always have more buffer than needed (will over-access for min. +1 byte)
	if(used == total) {
		total *= 2;
		buf = (char *) realloc(buf, total);
		gt_assert(buf != NULL, 23, "out of memory", assert_failed);
	}
	
	used += ifs.gcount();
	buf[used] = '\0';
	ifs.close();
	
	*buffer = buf;
	return used;
	
assert_failed:
	if(buf)
		free(buf);
	ifs.close();
	throw m_err;
}

std::string XmlTree::_trim(std::string s) {
	string::size_type beg = s.find_first_not_of(WHITESPACE);
	string::size_type end = s.find_last_not_of(WHITESPACE);
	string es;
	
	if(beg == string::npos)
		return es;
	else
		return s.substr(beg, (end == string::npos) ? string::npos : (end - beg + 1));
}

