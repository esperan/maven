/*
 *  maven Programming Language
 *  Created by Elliot Chance <elliot@chancemedia.com>.
 */

#include "maven.h"
#include "struct_MavenCompiler.h"
#include "strings.h"
#include "output.h"
#include "compiler.h"

void init(MavenCompiler* c) {
	// open the required file handles
	c->mapFileHandle.open((c->option_n + ".mmap").c_str());
	c->cppFileHandle.open((c->option_n + ".cpp").c_str());
	
	// test for bad file handles
	if(!c->mapFileHandle.is_open())
		die("Unable to open " + c->option_n + ".mmap for writing");
	if(!c->cppFileHandle.is_open())
		die("Unable to open " + c->option_n + ".cpp for writing");

	string mavencoreh = combinePaths(c->binDirectory, c->iniFile.getKey("directories.src")) +
									 "mavencore/mavencore.h";
	writeCPPLine(c, string("#include \"") + mavencoreh + "\"");
	writeCPPLine(c, string("#include \"") +
				 combinePaths(c->currentDirectory, c->option_n, false) + ".mmap\"");
	writeCPPLine(c, string("namespace ") + MAVEN_BARE_NAMESPACE + " {\n");
}
