/*
 *  maven Programming Language
 *  Created by Elliot Chance <elliot@mavenlang.com>.
 */

#ifndef MAVEN_STRUCTS
#define MAVEN_STRUCTS 1

#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <sys/types.h>
#include <dirent.h>

using namespace std;

enum MavenQuotes {
	MQ_NONE = 0,
	MQ_TICK = 1,
	MQ_SINGLE = 2,
	MQ_DOUBLE = 3
};

enum MavenComments {
	MC_NONE = 0,
	MC_SINGLE = 1,
	MC_MULTI = 2
};

enum MavenBrackets {
	MB_NAMESPACE = 0,
	MB_CLASS = 1,
	MB_FUNCTION = 2,
	MB_STATEMENT = 3
};

class MavenCompiler;

class StringList {
private:
	vector<string> list;
	
public:
	StringList();
	void push(string item);
	int length();
	string& operator [](int element);
	StringList operator =(string str);
	string join(string glue);
	string join(string glue, int length);
	string join(string glue, int start, int end);
	string pop();
	void clear();
	StringList setTypes(string str);
};

// when dissecting an entity this lets the parent dissector know if
// the left have side can be assigned. Usually this will be MM_OK,
// but in cases of 'readonly', 'constant' variables it may be
// protected and hence MU_CONSTANT.
// Entityies that are not storage mechanisms are given MU_IMPOSSIBLE,
// like:
//    myFunction() = 5;
enum MavenMutability {
	MU_OK = 0,
	MU_CONSTANT = 1,
	MU_IMPOSSIBLE = 2
};

class MavenDissectBlock {
public:
	string forceCast;
	string unary, postunary;
	string original, processed;
	StringList types;
	MavenMutability mutability;
	
public:
	MavenDissectBlock();
	void reset();
	void trim();
};

class MavenVariable {
public:
	string name;
	string type;
	string defaultValue;
	int atLine;
	bool isStatic;
	MavenMutability mutability;
	bool isPublic;
	string doc;
	bool isInherited;
	bool isExternal;
	
public:
	MavenVariable();
	MavenVariable(string _name, string _type);
	string getCPP(MavenCompiler* mc);
	void reset();
};

class MavenVariables {
private:
	vector<MavenVariable> vars;
	
public:
	MavenVariables();
	void push(MavenVariable var);
	int length();
	MavenVariable& operator[](int element);
	string getCPP(MavenCompiler* mc);
};

class MavenFunction {
public:
	string returnType;
	string name;
	MavenVariables args;
	string descArgs;
	int atLine;
	string doc;
	bool isPublic;
	bool isConstant;
	bool isStatic;
	bool isExternal;
	string alias;
	bool alias_system;
	bool isExported;
	bool isInherited;
	bool isOverride;
	
public:
	MavenFunction();
	string getCPPLine(MavenCompiler* mc, string className, bool isVirtual = false);
	string getSignature();
};

class MavenFunctions {
private:
	vector<MavenFunction> funcs;
	
public:
	MavenFunctions();
	void push(MavenFunction func);
	int length();
	MavenFunction& operator[](int element);
};

class MavenEnumItem {
public:
	string name;
	int value;
	bool isDefault;
	
public:
	MavenEnumItem();
};

class MavenEnum {
public:
	string name;
	vector<MavenEnumItem> items;
	
public:
	int getDefaultValue();
};

class MavenObject {
public:
	string name;
	bool isAbstract;
	bool isFinal;
	int line;
	string doc;
	MavenVariables variables;
	MavenFunctions functions;
	string extends;
	
public:
	MavenObject();
	void reset();
};

class MavenObjects {
private:
	vector<MavenObject> objects;
	
public:
	MavenObjects();
	int length();
	MavenObject& operator [](int element);
	void push(MavenObject object);
};

class MavenNamespace {
public:
	string name;
	string doc;
	MavenObjects objects;
	vector<MavenEnum> enums;
};

class MavenNamespaces {
private:
	vector<MavenNamespace> namespaces;
	
public:
	MavenNamespaces();
	int length();
	MavenNamespace& operator [](int element);
	void push(MavenNamespace object);
};

class MavenError {
public:
	string file;
	int line;
	int level;
	string message;
	string arg1, arg2;
	
public:
	MavenError();
};

class MavenErrors {
private:
	vector<MavenError> errors;
	
public:
	MavenErrors();
	int length();
	MavenError& operator[](int element);
	void push(MavenError error);
	void clear();
};

class MavenObjectDiscovery {
public:
	int namespaceID;
	int objectID;
	
public:
	MavenObjectDiscovery(int nID, int oID);
};

class MavenINIPair {
public:
	string k;
	string v;
};

class MavenINI {
protected:
	vector<MavenINIPair> ini;
	
public:
	MavenINI();
	bool setKey(string key, string value);
	string getKey(string key);
};

class MavenPair {
public:
	string key, value;
	
public:
	MavenPair(string k, string v);
};

class MavenCompiler {
public:
	/** Map file handle for generated .mmap */
	ofstream mapFileHandle;
	
	/** C++ file generated by the maven compiler. */
	ofstream cppFileHandle;
	
	/** Holds all object and variable information for all namespaces. */
	MavenNamespaces namespaces;
	
	/** Stack for errors. */
	MavenErrors errors;
	
	/** Current line number. */
	int lineNumber;
	
	/** Lines to be prepended when creating the main() function. */
	StringList entryPoint;
	
	/** Current maven file being processed. */
	string currentFile;
	
	/** Current active namespace. */
	string currentNamespace;
	
	/** Current active class name (bare name). */
	string currentClass;
	
	/** Current function. */
	string currentFunction;
	int currentBlockDepth;
	int dissectDepth;
	StringList reservedWords;
	vector<MavenVariables> localScope;
	vector<string> imports;
	string doc;
	vector<MavenBrackets> bracketStack;
	vector<MavenObjectDiscovery> discovery;
	int totalLines;
	int totalFiles;
	long start;
	
	//! objects to check for ObjectNilException and BoundsException
	vector<MavenPair> objectCheck;
	vector<string> postLines;
	vector<string> selectors;
	
	// this is collected throughout the dissection of the line then
	// prepended before flushing to the CPP file.
	string beforeLine;
	
	// filled when the MavenCompiler object is created.
	string currentDirectory;
	
	// filled with maven.ini before each maven compile.
	MavenINI iniFile;
	
	// all the extra .o's to include in the compilation.
	StringList extraLibraries;
	
	// set and reset with switch()
	string switchArgument;
	
	// the UID is a unique ID for naming temporary objects.
	int UID;
	
	// for use with the 'with' statement
	MavenVariable withObject;
	
	// the last statements, important for 'if' and 'do'
	string lastStatament;
	
public:
	// options
	bool   option_c;           // Compile object file but do not link
	bool   option_e;           // Execute after compilation.
	bool   option_h;           // Show help and quit.
	string option_i;           // Input file(s)
	bool   option_K;           // Keep CPP and MMAP
	string option_n;           // Result executable/object name.
	string option_o;           // Optimisation level (GNU specific.)
	bool   option_r;           // Show compilation statistics.
	int    option_s;           // Object-safe level
	bool   option_v;           // Print version and quit.
	bool   option_V;           // Show all compiler variables before compiling.
	string option_w;           // Supress specific warning.
	string option_W;           // Show specific warning.
	bool   option_x;           // Do not compile or link (no input file required.)
	string option_doc_sql;     // Place for SQL documentation.
	string option_doc_sqlite3; // Place for SQLite3 documentation.
	string option_doc_xml;     // Place for XML documentation.
	string option_base_path;   // where maven was originally executed from
	
public:
	MavenCompiler();
};

#endif
