#include <ctime>
#include <stdio.h>
#include <string.h>
#include "Utils.h"

using namespace std;

string now() {
	string ret;
	time_t t = time(NULL);
	struct tm* ti = localtime(&t);

	ret = asctime(ti);
	return trim(ret);
}

string trim(string s) {
	char kill[] = {'\t', ' ', '\r', '\n'};
	int i;
	bool hit = true;

	while(hit) {
		for(i = 0; i < sizeof(kill) / sizeof(char) && s[0] != kill[i]; i++);

		if(s[0] == kill[i])
			s.erase(0, 1);
		else
			hit = false;
	}

	hit = true;
	while(hit) {
		for(i = 0; i < sizeof(kill) / sizeof(char) && s[s.length() - 1] != kill[i]; i++);

		if(s[s.length() - 1] == kill[i])
			s.erase(s.length() - 1, 1);
		else
			hit = false;
	}

	return s;
}

string itos(int i) {
	char buf[30];
	string s;

	sprintf(buf, "%d", i);
	s = buf;
	return s;
}

string itoh(int i) {
	char buf[30];
	string s;

	sprintf(buf, "%02x", i);
	s = buf;
	return s;
}

bool stringListFind(list<string> l, string needle) {
	list<string>::iterator i;

	for(i = l.begin(); i != l.end(); i++)
		if((*i) == needle)
			return true;
	return false;
}
