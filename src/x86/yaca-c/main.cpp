#include <iostream>
#include <fstream>
#include "Template.h"
#include "Utils.h"
#include "Message.h"
#include "Globals.h"
#include "Source.h"

using namespace std;

#define TEMPLATE_PATH "/home/david/Code/yaca/src/x86/yaca-c/templates"

void run() {
	/*
	 * Bootstrap: Create export headers out of object exports, with the ability
	 * to use own exports (e.g. call remote messages of the same node type).
	 * This works by first defining out the remote message calls (MsgBootTemplate)
	 * Then, the object is compiled -> the object file is then parsed and
	 * the export header created. Then, the object is recompiled with a normal
	 * message call header (MsgNormalTemplate).
	 */

	string str("");

	Source src(Globals::getStr("nodeName") + ".C");
	Template tBoot(Globals::getStr("MsgBootTemplate"));
	ofstream f1(Globals::getStr("messageHeader").c_str(), ofstream::out);
	tBoot.addKey("<NODENAME>", Globals::getStr("nodeName"));
	f1 << tBoot.parse() << endl;
	f1.close();

	ofstream f2((str + "R" + Globals::getStr("nodeName") + ".h").c_str(), ofstream::out);
	f2 << endl;
	f2.close();
	ofstream f3((str + "R" + Globals::getStr("nodeName") + ".h").c_str(), ofstream::out);
	f3 << Message::createRemoteHeader(&src) << endl;
	f3.close();

	Template tNormal(Globals::getStr("MsgNormalTemplate"));
	ofstream f4(Globals::getStr("messageHeader").c_str(), ofstream::out);
	tNormal.addKey("<NODENAME>", Globals::getStr("nodeName"));
	f4 << tNormal.parse() << endl;
	f4.close();

	src.recompile();

	Template tSendCode(Globals::getStr("outTemplate"));
	ofstream f5((str + "R" + Globals::getStr("nodeName") + ".C").c_str(), ofstream::out);
	tSendCode.addKey("<CODE>", Message::createSendCode(&src));
	tSendCode.addKey("<CREATED>", now());
	tSendCode.addKey("<ORIGIN>", Globals::getStr("nodeName") + ".C");
	f5 << tSendCode.parse() << endl;
	f5.close();
	
	Source rsrc(str + "R" + Globals::getStr("nodeName") + ".C");
	rsrc.compile();
	
	Template tFunctionTable(Globals::getStr("outTemplate"));
	ofstream f6("ftable.C", ofstream::out);
	tFunctionTable.addKey("<CODE>", Message::createFunctionTable(&src));
	tFunctionTable.addKey("<CREATED>", now());
	tFunctionTable.addKey("<ORIGIN>", Globals::getStr("nodeName") + ".C");
	f6 << tFunctionTable.parse() << endl;
	f6.close();
	
	Source rft("ftable.C");
	rft.compile();
	
	
	XmlTree xmlt = Message::createXml(&src);
	xmlt.write((Globals::getStr("nodeName") + ".xml").c_str());
	
/*
	list<Message> l = src.getExports();
	list<Param> pars;
	list<Param>::iterator i;

	pars = l.front().getParams();

	cout << l.front().getName() << endl;

	for(i = pars.begin(); i != pars.end(); i++) {
		cout << (*i).getType() << " " << (*i).getName() << " (" << (*i).getSize() << "b, r" << (*i).getReg() << ")" << endl;
	} */ // FIXME until here



//	Message exp("SomeFunc");
/*	list<Param> pars;
	list<Param>::iterator i;*/
/*
	cout << "Getting..." << endl;

	pars = l.front().getParams();

	cout << "Got processed params" << endl;

	for(i = pars.begin(); i != pars.end(); i++) {
		cout << (*i).getType() << " " << (*i).getName() << " (" << (*i).getSize() << "b, r" << (*i).getReg() << ")" << endl;
	}*/
}

int main(int argc, char** argv) {
	string nodeName, param, es;
	int verbose = 0, i;
	Globals global;
	Source source;

	if(argc < 2) {
		cout << "yaca bake usage: %s <node name> [options]" << endl
			<< "Options include:" << endl << endl
			<< "-v    Increase verbosity level, can be used several times" << endl;
		return 0;
	}
	for(i = 2; i < argc; i++) {
		param = argv[i];
		if(param == "-v")
			verbose++;
		else
			cerr << "Warning: unknown option \"" << param << "\"" << endl;
	}
	if(verbose >= 1) {
		cout << "Info: verbosity level " << verbose << endl;
	}

	nodeName = argv[1];
	if(nodeName.find(".") != string::npos) {
		nodeName = nodeName.substr(0, nodeName.find("."));
		if(verbose >= 2) {
			cerr << "Warning: supplied a file name instead of a node name. A node name must not include the '.' character." << endl
				<< "Removing suffix and assuming node name \"" << nodeName << "\"" << endl;
		}/* else if(verbose >= 1) {
			cerr << "Warning: supplied a file name instead of a node name" << endl;
		}*/
	}

	Globals::setStr("tmpfile", "/tmp/yacac1"); // TODO: add PID here, below too
	Globals::setStr("tmpfile2", "/tmp/yacac2");
	Globals::setStr("sizeofTemplate", TEMPLATE_PATH "/sizeof.tpl");
	Globals::setStr("MsgNormalTemplate", TEMPLATE_PATH "/msg-normal.tpl");
	Globals::setStr("MsgBootTemplate", TEMPLATE_PATH "/msg-bootstrap.tpl");
	Globals::setStr("outTemplate", TEMPLATE_PATH "/out.tpl");
	Globals::setStr("messageHeader", "Messages.h");
	Globals::setStr("mcu", "atmega8");
	Globals::setStr("compilerOptions", "");
	Globals::setInt("verbose", verbose);
	Globals::setStr("nodeName", nodeName);
	Globals::setStr("canSendFunc", "CanSend");

	try {
		run();
	} catch(const char* err) {
		cerr << "Error: " << err << endl;
		if(!verbose)
			cerr << "Clueless? Try verbose mode - you can even issue -v multiple times." << endl;
	}

/*	source.setFile(nodeName + ".C");

	try {
		source.compile();
		cout << endl << "----------------- Symbol list -----------------" << endl << source.getSymbols() << endl;
	} catch(const char* err) {
		cerr << "Error: " << err << endl;
	}

	cout << endl << endl;

	cout << "sizeof(int) = " << Source::sizeOf("int") << endl;
	cout << "sizeof(int) = " << Source::sizeOf("int") << endl;
	cout << "sizeof(char) = " << Source::sizeOf("char") << endl;

	try {
		Template t("out.tpl");

		t.addKey("<CREATED>", now());
		t.addKey("<ORIGIN>", "SomeClass.cpp");
		t.addKey("<CODE>", "int main() {}");
		cout << t.parse() << endl;
	} catch(const char* err) {
		cerr << "Error: " << err << endl;
	}*/

	return 0;
}
