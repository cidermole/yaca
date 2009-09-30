#ifndef _UTILS_H_
#define _UTILS_H_

#include <string>
#include <list>

#define throwable() string thr("")
#define throw_error(s) throw (thr + s).c_str()
#define ES thr

using namespace std;

string now();
string trim(string s);
string itos(int i);
string itoh(int i);
bool stringListFind(list<string> l, string needle);

#endif /* _UTILS_H_ */

