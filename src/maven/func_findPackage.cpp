/*
 *  maven Programming Language
 *  Created by Elliot Chance <elliot@mavenlang.com>.
 */

#include "maven.h"
#include "compiler_strings.h"
#include "installer.h"

string findPackage(string package) {
	StringList files = readDirectory(".");
	
	// as a local exact name
	for(int i = 0; i < files.length(); ++i) {
		if(files[i] == package)
			return package;
	}
	
	// as a local .zip
	for(int i = 0; i < files.length(); ++i) {
		if(files[i] == package + ".zip")
			return package + ".zip";
	}
	
	// can't find it
	return "";
}
