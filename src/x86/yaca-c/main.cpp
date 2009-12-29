#include <iostream>
#include <assert.h>
#include <fstream>
#include "Template.h"
#include "Utils.h"
#include "Message.h"
#include "Globals.h"
#include "Source.h"
#include "../yaca-path.h"

using namespace std;


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
	xmlt.write((Globals::getStr("nodeName") + ".nds").c_str());
	
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
	string nodeName, param, es, cmd, configFile;
	int verbose = 0, i;
	bool newMode = false, cleanMode = false, saveTemps = false;
	Globals global;
	Source source;

	init_yaca_path();

	if(argc < 2) {
		cout << "yaca-c usage: %s <node name> [options]" << endl
			<< "Options include:" << endl << endl
			<< "-v    Increase verbosity level, can be used several times" << endl
			<< "-new <config xml>: Programs a new node" << endl
			<< "-clean" << endl
			<< "-save-temps" << endl;
		return 0;
	}
	for(i = 2; i < argc; i++) {
		param = argv[i];
		if(param == "-v")
			verbose++;
		else if(param == "-new") {
			assert(i + 1 < argc);
			newMode = true;
			configFile = argv[i + 1];
			i++;
		} else if(param == "-clean")
			cleanMode = true;
		else if(param == "-save-temps")
			saveTemps = true;
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
	Globals::setStr("sizeofTemplate", es + yaca_path + "/src/x86/yaca-c/templates/sizeof.tpl");
	Globals::setStr("MsgNormalTemplate", es + yaca_path + "/src/x86/yaca-c/templates/msg-normal.tpl");
	Globals::setStr("MsgBootTemplate", es + yaca_path + "/src/x86/yaca-c/templates/msg-bootstrap.tpl");
	Globals::setStr("outTemplate", es + yaca_path + "/src/x86/yaca-c/templates/out.tpl");
	Globals::setStr("messageHeader", "Messages.h");
	Globals::setStr("mcu", "atmega8");
	Globals::setStr("compilerOptions", es + "-I" + yaca_path + "/build/include");
	Globals::setInt("verbose", verbose);
	Globals::setStr("nodeName", nodeName);
	Globals::setInt("saveTemps", saveTemps ? 1 : 0);

	if(!cleanMode) {
		try {
			run();

			cmd = es + "avr-gcc -o " + nodeName + "-app.o -L" + yaca_path + "/build/embedded/lib R" + nodeName + ".o " + nodeName + ".o ftable.o -Wl,-T " + yaca_path + "/src/x86/yaca-c/link-" + Globals::getStr("mcu") + ".txt -lyaca";
			if(verbose > 1)
				cout << "Info: running command \"" << cmd << "\"" << endl;
			if(system(cmd.c_str()))
				throw "Linking failed";
			cmd = es + "avr-objcopy -O ihex -R .eeprom " + nodeName + "-app.o " + nodeName + "-app.hex";
			if(verbose > 1)
				cout << "Info: running command \"" << cmd << "\"" << endl;
			if(system(cmd.c_str()))
				throw "Object copying failed";
			
			// TODO: make option to flash from here
		
			if(newMode) {
				cmd = es + yaca_path + "/build/bin/yaca-hexmerge " + yaca_path + "/build/embedded/bootloader/bootloader.hex " + nodeName + "-app.hex " + nodeName + "-full.hex";
				if(verbose > 1)
					cout << "Info: running command \"" << cmd << "\"" << endl;
				if(system(cmd.c_str()))
					throw "Hex merge failed";
				cmd = es + yaca_path + "/build/bin/yaca-program " + nodeName + ".nds " + configFile + " -new `" + yaca_path + "/build/bin/yaca-flash 0 " + nodeName + "-app.hex -crc` " + nodeName + "-app.eep";
				if(verbose > 1)
					cout << "Info: running command \"" << cmd << "\"" << endl;
				if(system(cmd.c_str()))
					throw "yaca-program failed";

				// TODO: flash stuff (avrdude)
			}
		} catch(const char* err) {
			cerr << "Error: " << err << endl;
			if(!verbose)
				cerr << "Clueless? Try verbose mode - you can even issue -v multiple times." << endl;
		}
	}
	
	if(verbose)
		cout << "Info: cleaning up" << endl;
	if(!saveTemps) {
		system((es + "rm -f R" + nodeName + ".*").c_str());
		system((es + "rm -f " + nodeName + "*.o").c_str());
		system("rm -f ftable.*");
	}
	
	if(cleanMode) {
		system((es + "rm -f " + nodeName + "-app.hex").c_str());
		system((es + "rm -f " + nodeName + "-full.hex").c_str());
		system((es + "rm -f " + nodeName + "-app.eep").c_str());
		system((es + "rm -f " + nodeName + ".nds").c_str());
		system((es + "rm -f Messages.h").c_str());
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
