#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <cctype>
#include <algorithm>
#include "Message.h"
#include "Source.h"
#include "Utils.h"
#include "Globals.h"
#include "Template.h"

using namespace std;

void Message::addParam(string type) {
	string s("");
	addParam(type, s + "p" + itos(m_params.size()));
}

void Message::addParam(string type, string name) {
	throwable();
	int size = Source::sizeOf(type);
	Param np;

	if(size % 2) {
		m_regPlace -= size + 1;
		m_packStyle |= (1 << m_packIndex);
		m_packIndex++;
	} else {
		m_regPlace -= size;
		m_packIndex += size;
	}

	if(m_regPlace < 8) {
		if(Globals::getInt("verbose") >= 1) {
			cerr << "Error: Register underrun while figuring out register in which argument" << endl
				<< "Error: \"" << name << "\" is passed: Current amount of parameters in Message" << endl
				<< "Error: \"" << m_name << "\" implies that some parameters should be handed over" << endl
				<< "Error: on the stack. As this case should be rare with Message functions, it is" << endl
				<< "Error: currently not implemented. See the following URL for details on gcc reg" << endl
				<< "Error: usage on AVR devices:" << endl
				<< "Error: http://www.roboternetz.de/wissen/index.php/Avr-gcc/Interna" << endl;
		}
		throw_error("Passing params via stack isn't implemented (passing reg dropping below 8)");
	}

	m_size += size;
	if(m_size > 12) { // 8 + implicit CAN ID
		if(Globals::getInt("verbose") >= 1) {
			cerr << "Error: Overall size of Message parameters must not exceed 8 bytes." << endl
				<< "Error: This is the maximum amount of data that can be put in a " << endl
				<< "Error: single CAN frame." << endl;
		}
		throw_error("Message param size overrun in function \"" + m_name + "\" on param \"" + name + "\"");
	}

	np.init(type, name, size, m_regPlace);
	m_params.push_back(np);
}

string Message::createRemoteHeader(Source* src) {
	stringstream header(stringstream::out);
	stringstream rtrs(stringstream::out);
	list<Message> l = src->getExportsForR();
	list<Message>::iterator i;
	list<Param> params;
	list<Param>::iterator j;
	Template t(Globals::getStr("outTemplate"));

	header << "#include \"Messages.h\"" << endl << endl;
	header << "namespace R" << Globals::getStr("nodeName") << " {" << endl;

	for(i = l.begin(); i != l.end(); i++) {
		if((*i).getType() == RTR) {
			rtrs << "\tvoid HDR(" << (*i).getName() << "());" << endl;
		} else {
			header << "\tvoid HDM(" << (*i).getName() << "(";

			params = (*i).getParams();
			for(j = params.begin(); j != params.end(); j++)
				header << (j == params.begin() ? "" : ", ") << (*j).getType();

			header << "));" << endl;
		}
	}

	header << endl << endl << "\t// ************* Remote Transmission Requests *************" << endl << endl;
	header << rtrs.str();

	header << "}" << endl;

	t.addKey("<CREATED>", now());
	t.addKey("<ORIGIN>", Globals::getStr("nodeName") + ".C");
	t.addKey("<CODE>", header.str());
	return t.parse();
}

void Message::_createInlineAsm(stringstream& ss, Param param) {
	int i;

	for(i = param.getReg() + param.getSize() - 1; i >= param.getReg(); i--)
		ss << "\t\t\"push r" << itos(i) << " ; " << param.getName() << " B" << itos(i - param.getReg()) << "\" \"\\n\\t\"" << endl;
}

string Message::createSendCode(Source* src) {
	stringstream code(stringstream::out);
	stringstream includes(stringstream::out);
	list<string> includeList;
	list<Message> l = src->getSends();
	list<Message>::iterator i;
	list<Param> params;
	list<Param>::iterator j;
	list<Param>::reverse_iterator ri;

	for(i = l.begin(); i != l.end(); i++) {
		if(!stringListFind(includeList, (*i).getNameSpace())) {
			includes << "#include \"" << (*i).getNameSpace() << ".h\"" << endl;
			includeList.push_back((*i).getNameSpace());
		}

		code << endl << "void __attribute__ ((naked, noinline)) " << (*i).getNameSpace() << "::" <<
			(((*i).getType() == RTR) ? "__rtr_" : "__msg_")
			<< (*i).getName() << "(";

		params = (*i).getParams();
		for(j = params.begin(); j != params.end(); j++)
			code << (j == params.begin() ? "" : ", ") << (*j).getType() << " " << (*j).getName();

		code << ") {" << endl;

		code << "\tasm volatile(" << endl
		<< "\t\t\"ldi r31, 0x" << itoh((*i).getPackStyle()) << "\" \"\\n\\t\" // packstyle: " << (*i).getPackStyle() << endl
		<< "\t\t\"rjmp _pack_n_go\" \"\\n\\t\"" << endl
		<< "\t:::\"r31\");" << endl;
		code << "}" << endl;
	}

	return (includes.str() + code.str());
}

string Message::createFunctionTable(Source* src) {
	stringstream code(stringstream::out);
	list<Message> l = src->getExports();
	list<Message>::iterator i;
	int count;

	code << "#include <inttypes.h>\n";
	code << "#include <avr/pgmspace.h>\n";
	code << "#include \"" << Globals::getStr("nodeName") << ".h\"\n\n";
	// FIXME should include <yaca.h> instead. needs command adjustment though
	code << "extern \"C\" {\n";
	code << "\ttypedef struct {\n";
	code << "\t\tuint8_t packstyle;\n";
	code << "\t\tuint8_t flags;\n";
	code << "\t\tvoid *fp;\n";
	code << "\t} fpt_t;\n";
	code << "}\n\n";
	
	code << "extern fpt_t fpt[] PROGMEM;\n" << "extern uint8_t fpt_size PROGMEM;\n\n";
	
	code << "fpt_t fpt[] = {\n";

	for(i = l.begin(), count = 0; i != l.end(); i++, count++) {
		code << (count ? ",\n\t" : "\t") << "{0x" << itoh((*i).getPackStyle()) << ", " << ((i->getType() == RTR) ? "0x01" : "0x00")
			<< ", (void *)" << Globals::getStr("nodeName") << "::" << (((*i).getType() == RTR) ? "__rtr_" : "__msg_") << (*i).getName() << "}";
	}

	code << "\n};\n";
	code << "uint8_t fpt_size = " << count << ";" << endl;

	return code.str();
}

XmlTree *Message::makeEepromDescriptorNode() {
	throwable();
	ifstream ifs;
	string line, name, value, type;
	size_t pos, ws;
	int nline = 1;
	XmlTree *xt = new XmlTree("eeprom");
	XmlTree *xtmp;
	
	ifs.open((Globals::getStr("nodeName") + ".h").c_str(), ifstream::in);
	if(!ifs.good())
		throw_error("Can't open header file \"" + Globals::getStr("nodeName") + ".h\" for reading!");
	
	while(getline(ifs, line)) {
		if(line.find("#define") != string::npos) {
			if((pos = line.find("YC_EE_")) != string::npos) {
				if((ws = line.find(" ", pos)) == string::npos)
					goto err_;
				
				name = line.substr(pos + 6, ws - pos - 6);
				transform(name.begin(), name.end(), name.begin(), (int(*)(int)) tolower);
				
				if((pos = line.find("(")) == string::npos)
					goto err_;
				if((ws = line.find(")")) == string::npos)
					goto err_;
				value = line.substr(pos + 1, ws - pos - 1);
				
				if((pos = line.find("// ")) == string::npos)
					goto err_;
				ws = line.find(" ", pos + 3);
				ws = (ws != string::npos) ? (ws - pos - 3) : string::npos;
				type = line.substr(pos + 3, ws);
				
				xtmp = new XmlTree("setting");
				xtmp->setAttribute("name", name);
				xtmp->setAttribute("address", value);
				xtmp->setAttribute("type", type);
				xt->append(xtmp);
			}
		}
		nline++;
	}
	
	ifs.close();
	return xt;
	
err_:
	if(Globals::getInt("verbose") >= 1)
		cerr << "Info: Correct syntax example for the following error: #define YC_EE_SETTINGNAME YE(10) // int32" << endl <<
				"Info: (#define<SPACE>YC_EE_<SETTING NAME> YE(<DECIMAL ADDRESS IN EEPROM>) //<SPACE><TYPE><NEWLINE>" << endl;
	throw_error("Line " + itos(nline) + " of \"" + Globals::getStr("nodeName") + ".h" + "\" looks very much like an eeprom descriptor, but its syntax is wrong.");
	return NULL;
}

XmlTree Message::createXml(Source* src) {
//	list<Message> exps = src->getExports(); // export = msg-in = incoming message handled by function. fn is exported
//	list<Message> snds = src->getSends(); // send = msg-out = transmitting a message with function params.
	list<Message> proc;
	list<Message>::iterator i;
	list<Param>::iterator pi;
	list<Param> lp;
	int j;
	
	XmlTree xnds("nds");
	XmlTree *xin = new XmlTree("messages-in"); // will be freed by xnds
	XmlTree *xout = new XmlTree("messages-out"); // will be freed by xnds
	xnds.append(xin);
	xnds.append(xout);
	xnds.append(makeEepromDescriptorNode());

	for(j = 0; j < 2; j++) {
		if(j == 0)
			proc = src->getExports();
		else
			proc = src->getSends();
		
		for(i = proc.begin(); i != proc.end(); i++) {
			XmlTree *xmsg = new XmlTree("msg"); // will be freed by xin or xout
			xmsg->setAttribute("name", i->getName().c_str());
			lp = i->getParams();
			for(pi = lp.begin(); pi != lp.end(); pi++)
				xmsg->append(new XmlTree("param", pi->getType().c_str()));
			
			if(j == 0)
				xin->append(xmsg);
			else
				xout->append(xmsg);
		}
	}
	
	// XXX: may not work in all cases (destructor deletes pointers, copying the class wouldn't work)
	return xnds;
}






