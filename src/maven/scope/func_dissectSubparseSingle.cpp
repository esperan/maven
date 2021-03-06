/*
 *  maven Programming Language
 *  Created by Elliot Chance <elliot@chancemedia.com>.
 */

#include "maven.h"
#include "struct_StringList.h"
#include "struct_MavenCompiler.h"
#include "strings.h"
#include "constants.h"
#include "objects.h"
#include "operators.h"
#include "struct_MavenObject.h"
#include "errors.h"
#include "keywords.h"
#include "variables.h"
#include "namespaces.h"
#include "scope.h"
#include "output.h"
#include "compiler_functions.h"

string dissectSubparseSingle(MavenCompiler* c, string code, string& r, StringList& types,
							 MavenMutability& mut, MavenVariable& prev, bool lastSubparse,
							 string nextOp, StringList argumentTypes, bool warn) {
	code = trim(code);
	
	//cout << "force cast = '" << 
	
	// a maven.String
	if(isString(code)) {
		types.setTypes("maven.String");
		r = "(new maven::String(" + code + "))";
		return r;
	}
	
	// just grouping brackets
	if(code[0] == '(') {
		r = "(" + dissectCode(c, code.substr(1, code.length() - 2), types, mut) + ")";
		return r;
	}
	
	// chop up
	int findArguments = code.find('('), findElement = code.find('[');
	string newObject = "";
	string newArguments = "";
	
	if(findArguments != string::npos) {
		// bug #60: find_last_of is not appropraite for separating dots
		newArguments = trim(code.substr(findArguments + 1, code.find_last_of(')') - findArguments - 1));
		MavenMutability argumentsMut;
		newArguments = dissectCode(c, newArguments, argumentTypes, argumentsMut);
		newObject = trim(code.substr(0, findArguments));
	}
	
	string newElement = "";
	if(findElement != string::npos)
		// bug #60: find_last_of is not appropraite for separating dots
		newElement = trim(code.substr(findElement + 1, code.find_last_of(']') - findElement - 1));
	
	if(findArguments == string::npos && findElement == string::npos)
		newObject = trim(code);
	else if(findArguments == string::npos && findElement != string::npos)
		newObject = trim(code.substr(0, findElement));
	else if(findArguments != string::npos && findElement != string::npos)
		newObject = trim(code.substr(0, min(findElement, findArguments)));
	
	// a single byte or multibyte characters is converted into its raw int form
	if(code[0] == '\'') {
		if(code.length() <= 4) {
			// if the char constant is upto 2 bytes it's considered a normal char
			types.setTypes("char");
			r = "(char)" + intToString(charToInt(code.substr(1, code.length() - 2)));
			return r;
		} else if(code.length() <= 6) {
			// if the char constant is upto 4 bytes it's considered an integer
			types.setTypes("int");
			r = "(int)" + intToString(charToInt(code.substr(1, code.length() - 2)));
			return r;
		} else {
			// any bigger becomes a problem, tell the user we are only going to be
			// using the first 4 chars
			types.setTypes("int");
			if(warn)
				pushWarning(c, "Only using first 4 bytes of character constant");
			r = "(int)" + intToString(charToInt(code.substr(1, 4)));
			return r;
		}
	}
	
	// setup firstWord variable
	string firstWord = "";
	for(int i = 0; i < code.length(); ++i) {
		if(isspace(code[i]))
			break;
		else firstWord += code[i];
	}
	
	// create an object
	if(firstWord == "new") {
		mut = MU_OK;
		r = "0";
		string newType = "";
		r = keywordNew(c, trim(newObject.substr(3)), newElement, newArguments, newType);
		types.setTypes(newType);
		return r;
	}
	
	MavenVariable resolve;
	bool isLocal;
	int namespaceID, objectID;
	if(prev.type == "") {
		
		if(findArguments == string::npos) {
			r += resolveVariable(c, newObject, resolve, namespaceID, objectID, isLocal, false);
			prev = resolve;
			string objectPath = cType(findObjectPath(c, stripRawType(prev.type), true));
			//if(!isDataType(objectPath) && resolve.type.length() > 0 && resolve.type[0] != '<' && nextOp != "=")
			//	r = "(" + objectPath + ")" + r;
			if(newElement != "")
				r += "->a[" + newElement + "]";
			//if(resolve.type.length() > 0 && resolve.type[0] != '<')
			//	r = "(" + r + ")";
		} else {
			
			// perhaps a compiler function
			// bug #59: Incomplete, this code needs to be enabled again.
			if(newObject[0] == '@') {
				/*if(newObject == "@library") {
				 // bug #62: this needs to be smarter
				 args = trim(args);
				 c->extraLibraries.push(combinePaths(c->binDirectory, c->iniFile.getKey("directories.lib")) +
				 args.substr(19, args.length() - 21));
				 return "";
				 } else*/
				if(newObject == "@selector") {
					string tempType = MAVEN_INVALID;
					r = compilerFunctionSelector(c, argumentTypes.join(","),
												 newArguments.substr(1, newArguments.length() - 2), tempType);
					types.push(tempType);
				} else if(newObject == "@type") {
					r = compilerFunctionType(c, argumentTypes.join(","));
				} else {
					r = MAVEN_INVALID;
					pushError(c, "Unknown compiler function %s", newObject);
				}
				return r;
			}
			
			int nID = findNamespaceID(c, c->currentNamespace);
			int oID = findObjectID(c, nID, "nil");
			bool found = false;
			for(int i = 0; i < c->namespaces->at(nID).objects->at(oID)->functions->length(); ++i) {
				// bug #61: check signature (pending)
				if(c->namespaces->at(nID).objects->at(oID)->functions->at(i).name == newObject &&
				   canCastBetween(c, argumentTypes.join(","),
								  c->namespaces->at(nID).objects->at(oID)->functions->at(i).getSignature(),
								  false, true)) {
					r += "(" + c->currentNamespace + "::nil::" + newObject + "(" + newArguments + "))";
					prev.type = c->namespaces->at(nID).objects->at(oID)->functions->at(i).returnType;
					prev.name = "<RETURNED>";
					resolve = prev;
					found = true;
					break;
				}
			}
			
			if(!found) {
				r = MAVEN_INVALID;
				pushError(c, "Can not find function %s(%s)", newObject, argumentTypes.join(","));
				return r;
			}
		}
	} else {
		if(findArguments == string::npos) {
			
			if(prev.type.substr(0, 7) == "<Class:")
				prev.type = prev.type.substr(8, prev.type.length() - 9);
			
			findClass(c, prev.type, namespaceID, objectID);
			smartAssert(namespaceID >= 0);
			smartAssert(objectID >= 0);
			
			bool found = false;
			for(int i = 0; i < c->namespaces->at(namespaceID).objects->at(objectID)->variables->length(); ++i) {
				if(c->namespaces->at(namespaceID).objects->at(objectID)->variables->at(i).name == newObject) {
					resolve = prev = c->namespaces->at(namespaceID).objects->at(objectID)->variables->at(i);
					found = true;
					
					string joiner = "->";
					if(c->namespaces->at(namespaceID).objects->at(objectID)->variables->at(i).isStatic) {
						r = c->namespaces->at(namespaceID).name + "::" +
							c->namespaces->at(namespaceID).objects->at(objectID)->name + "$static";
						joiner = "::";
					}
					if(!isDataType(c->namespaces->at(namespaceID).objects->at(objectID)->variables->at(i).type) &&
					   !lastSubparse)
						pushObjectSafety(c, r + joiner + prev.name);
					if(newElement != "")
						pushArraySafety(c, r + joiner + prev.name, newElement);
					//r = "(" + cType(findObjectPath(c, stripRawType(prev.type), true)) + ")";
					r += joiner + prev.name;
					if(newElement != "")
						r += "->a[" + newElement + "]";
					r = "(" + r + ")";
					break;
				}
			}
			
			// bug #58: need to show candidates, see below
			if(!found) {
				r = MAVEN_INVALID;
				pushError(c, "%s does not have the member variable %s", prev.name, newObject);
				return r;
			}
			
		} else {
			
			if(prev.type.substr(0, 7) == "<Class:")
				prev.type = prev.type.substr(8, prev.type.length() - 9);
			
			findClass(c, prev.type, namespaceID, objectID);
			int found = 0;
			for(int i = 0; i < c->namespaces->at(namespaceID).objects->at(objectID)->functions->length(); ++i) {
				// bug #61: check signature (pending)
				if(c->namespaces->at(namespaceID).objects->at(objectID)->functions->at(i).name == newObject) {
					found = 1;
					if(canCastBetween(c, argumentTypes.join(","),
									  c->namespaces->at(namespaceID).objects->at(objectID)->functions->at(i).getSignature(),
									  false, true)) {
						prev.name = "<RETURNED>";
						prev.type = c->namespaces->at(namespaceID).objects->at(objectID)->functions->at(i).returnType;
						resolve = prev;
						found = 2;
						
						string joiner = "->";
						if(c->namespaces->at(namespaceID).objects->at(objectID)->functions->at(i).isStatic)
							joiner = "::";
						string objectPath = findObjectPath(c, stripRawType(prev.type), true);
						if(!isDataType(objectPath))
							r = "((" + cType(objectPath) + ")" + r + ")";
						r += joiner;
						
						// deal with an alias
						if(c->namespaces->at(namespaceID).objects->at(objectID)->functions->at(i).alias != "")
							r = c->namespaces->at(namespaceID).objects->at(objectID)->functions->at(i).alias;
						else r += c->namespaces->at(namespaceID).objects->at(objectID)->functions->at(i).name;
						r += "(" + newArguments + ")";
						break;
					}
				}
			}
			
			// if none of those worked then perhaps we can find a varargs
			if(found != 2) {
				for(int i = 0; i < c->namespaces->at(namespaceID).objects->at(objectID)->functions->length(); ++i) {
					int argsLen = c->namespaces->at(namespaceID).objects->at(objectID)->functions->at(i).args.length();
					
					// no point looking at methods that take no arguments
					if(argsLen == 0)
						continue;
					
					// the method name must match
					if(c->namespaces->at(namespaceID).objects->at(objectID)->functions->at(i).name != newObject)
						continue;
					
					// the last parameter must be named 'varargs'
					if(c->namespaces->at(namespaceID).objects->at(objectID)->functions->at(i).args[argsLen - 1].name != "varargs")
						continue;
					
					// make sure the first part of the signature matches
					string fSig2 = c->namespaces->at(namespaceID).objects->at(objectID)->functions->at(i).getSignature();
					fSig2 = fSig2.substr(0, fSig2.find_last_of(','));
					int matchingArguments = 1; // FIXME: make this dynamic
					
					if(canCastBetween(c, argumentTypes.join(",", 0, argsLen - 2), fSig2, false, true)) {
						prev.name = "<RETURNED>";
						prev.type = c->namespaces->at(namespaceID).objects->at(objectID)->functions->at(i).returnType;
						resolve = prev;
						found = 2;
						
						string joiner = "->";
						if(c->namespaces->at(namespaceID).objects->at(objectID)->functions->at(i).isStatic)
							joiner = "::";
						string objectPath = cType(findObjectPath(c, stripRawType(prev.type), true));
						if(isDataType(objectPath))
							r += joiner;
						else r = "(" + objectPath + ")" + r + joiner;
						
						// deal with an alias
						if(c->namespaces->at(namespaceID).objects->at(objectID)->functions->at(i).alias != "")
							r = c->namespaces->at(namespaceID).objects->at(objectID)->functions->at(i).alias;
						else r += c->namespaces->at(namespaceID).objects->at(objectID)->functions->at(i).name;
						
						string nextTempVar = getNextTempVariable(c);
						StringList newNewArguments = splitCommas(newArguments);
						r += "(" + newNewArguments.join(",", 0, matchingArguments - 1) + ", " + nextTempVar + ")";
						
						// create the varargs
						c->beforeLine += "maven::objectArray* " + nextTempVar + " = new maven::objectArray(" +
							intToString(argumentTypes.length() - matchingArguments) + ");\n";
						
						StringList splitNewArguments = splitCommas(newArguments);
						for(int a = 0; a < argumentTypes.length() - matchingArguments; ++a) {
							c->beforeLine += nextTempVar + "->a[" + intToString(a) + "] = (maven::Object*) ";
							
							string nToO = argumentTypes[a + matchingArguments];
							if(isDataType(nToO)) {
								c->beforeLine += "new " + findObjectPath(c, nativeToObject(nToO), false) +
									"(" + splitNewArguments[a + matchingArguments] + ");\n";
							} else c->beforeLine += splitNewArguments[a + matchingArguments] + ";\n";
						}
						break;
					}
				}
			}
			
			if(found == 0) {
				r = MAVEN_INVALID;
				pushError(c, "%s does not have the member method %s", prev.name, newObject + "(" +
						  argumentTypes.join(",") + ")");
				return r;
			} else if(found == 1) {
				r = MAVEN_INVALID;
				// bug #58: need to show candidates
				//if(prev.name[0] == '<')
				pushError(c, "%s does not have the member method to match %s", prev.name, newObject +
						  "(" + argumentTypes.join(",") + ")");
				return r;
			}
		}
	}
	
	// make sure it was resolved
	if(r == MAVEN_INVALID) {
		types.setTypes(MAVEN_INVALID);
		mut = MU_IMPOSSIBLE;
		return r;
	}
	
	if(newElement != "")
		types.setTypes(stripRawType(resolve.type));
	else types.setTypes(resolve.type);
	mut = resolve.mutability;
	if(!isAssignOperator(nextOp)) {
		string temp = r, temp2 = cType(findObjectPath(c, types[0], true));
		//if(temp2.substr(0, 1) != "<")
		//   r = "(" + r + ")";
	}
	return r;
}
