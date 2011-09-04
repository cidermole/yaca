#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include <string.h>
#include <list>

#define throwable() string thr("")
//#define throw_error(s) throw (thr + s).c_str()
#define throw_error(s) \
	do { \
		char __errbuf[100*1024]; \
		strncpy(__errbuf, (thr + s).c_str(), 100*1024-1); \
		__errbuf[100*1024-1] = '\0'; \
		throw __errbuf; \
	} while(0)
#define ES thr

using namespace std;

string now();
string trim(string s);
string itos(int i);
string itoh(int i);
bool stringListFind(list<string> l, string needle);

#endif /* _UTILS_H_ */

