#include "Globals.h"

map<string, IntelliPtr> Globals::m_vars;

Globals::~Globals() {
	map<string, IntelliPtr>::iterator mi;

	for(mi = m_vars.begin(); mi != m_vars.end(); mi++)
		(*mi).second.destroy();
}
