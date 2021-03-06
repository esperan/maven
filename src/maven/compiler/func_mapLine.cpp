/*
 *  maven Programming Language
 *  Created by Elliot Chance <elliot@chancemedia.com>.
 */

#include "maven.h"
#include "struct_MavenCompiler.h"
#include "struct_StringList.h"
#include "strings.h"
#include "keywords.h"
#include "variables.h"
#include "namespaces.h"
#include "errors.h"
#include "doc.h"
#include "objects.h"
#include "functions.h"
#include "compiler.h"

bool mapLine(MavenCompiler* c, string line) {
	// ignore blank lines
	line = trim(line);
	if(line == "")
		return true;
	
	// we split the line up into smart tokens
	StringList tokens = smartTokens(line);
	
	// this could be an import
	if(tokens[0] == "import") {
		// we might need to import all
		if(tokens[1].find("*") != string::npos)
			return importAll(c, tokens[1], false);
		
		string importlocation = locateImport(c, tokens[1]);
		if(alreadyImported(c, importlocation))
			return true;
		
		string thisFile = c->currentFile;
		string thisNamespace = c->currentNamespace;
		int thisLine = c->lineNumber;
		c->imports->push(importlocation);
		++c->totalFiles;
		compileFile(c, importlocation, MAVEN_ONLY_MAP);
		return resetCurrent(c, thisFile, thisNamespace, thisLine);
	}
	
	// this could be a namespace
	if(tokens[0] == "namespace") {
		if(tokens.length() == 1) {
			pushError(c, "Namespace name expected after 'namespace'");
			return false;
		}
		pushNamespace(c, tokens[1]);
		return true;
	}
	
	// this could be registering a new variable
	MavenVariable var;
	if(isRegisteringVariable(c, tokens))
		return registerVariable(c, tokens, false, var);
	else if(tokens[0] == "external") {
		// external can mean a function or a variable
		if(line.find('(') == string::npos) {
			return registerVariable(c, tokens, false, var);
		} else {
			MavenFunction func = dissectFunction(c, line);
			if(func.name != "") {
				func.doc = filterDoc(c, c->doc);
				pushFunction(c, findNamespaceID(c, c->currentNamespace),
							 findObjectID(c, findNamespaceID(c, c->currentNamespace), c->currentClass), func);
				c->doc = "";
				return true;
			}
		}
	}
	
	// I don't know what this is, let the compiler check it
	return true;
}
