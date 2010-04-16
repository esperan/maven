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

/**
 * When dissecting an entity this lets the parent dissector know if the left have side can be assigned.
 * Usually this will be MM_OK, but in cases of 'readonly', 'constant' variables it may be protected and
 * hence MU_CONSTANT. Entityies that are not storage mechanisms are given MU_IMPOSSIBLE, like:
 * @code
 * myFunction() = 5;
 * @endcode
 */
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

struct MavenDocTag {
	
	/**
	 * @brief The main body of the documentation.
	 */
	string body;
	
	/**
	 * @brief \@brief.
	 */
	string tagBrief;
	
	/**
	 * @brief \@param.
	 */
	StringList tagParam;
	
	/**
	 * @brief \@return.
	 */
	string tagReturn;
	
	/**
	 * @brief \@since.
	 */
	string tagSince;
	
	/**
	 * @brief \@throws.
	 */
	StringList tagThrows;
	
	/**
	 * @brief \@version.
	 */
	string tagVersion;
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
	MavenDocTag doc;
	bool isInherited;
	bool isExternal;
	
public:
	MavenVariable();
	MavenVariable(string _name, string _type);
	string getCPP(MavenCompiler* mc);
	void reset();
	string getAnchorID();
};

struct MavenVariables {
	vector<MavenVariable> vars;
	
	MavenVariables();
	
	void push(MavenVariable var);
	int length();
	MavenVariable& operator[](int element);
	MavenVariable& at(int element);
	string getCPP(MavenCompiler* mc);
};

class MavenFunction {
public:
	string returnType;
	string name;
	MavenVariables args;
	string descArgs;
	int atLine;
	MavenDocTag doc;
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
	string getAnchorID();
};

struct MavenFunctions {
	vector<MavenFunction> funcs;
	
	MavenFunctions();
	
	void push(MavenFunction func);
	int length();
	MavenFunction& operator[](int element);
	MavenFunction& at(int element);
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

struct MavenObject {
	string name;
	bool isAbstract;
	bool isFinal;
	int line;
	MavenDocTag doc;
	MavenVariables* variables;
	MavenFunctions* functions;
	string extends;
	
	MavenObject();
	~MavenObject();
	void reset();
};

class MavenObjects {
private:
	vector<MavenObject*> objects;
	
public:
	MavenObjects();
	int length();
	MavenObject* operator [](int element);
	MavenObject* at(int element);
	void push(MavenObject* object);
};

struct MavenNamespace {
	string name;
	MavenDocTag doc;
	MavenObjects* objects;
	vector<MavenEnum> enums;
	
	MavenNamespace();
	~MavenNamespace();
};

struct MavenNamespaces {
	vector<MavenNamespace> namespaces;
	
	MavenNamespaces();
	
	int length();
	MavenNamespace& operator [](int element);
	MavenNamespace& at(int element);
	void push(MavenNamespace* ns);
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
	/**
	 * @brief Map file handle for generated .mmap
	 */
	ofstream mapFileHandle;
	
	/**
	 * @brief C++ file generated by the maven compiler.
	 */
	ofstream cppFileHandle;
	
	/**
	 * @brief Holds all object and variable information for all namespaces.
	 */
	MavenNamespaces* namespaces;
	
	/**
	 * @brief Stack for errors.
	 */
	MavenErrors errors;
	
	/**
	 * @brief Current line number.
	 */
	int lineNumber;
	
	/**
	 * @brief Lines to be prepended when creating the main() function.
	 */
	StringList entryPoint;
	
	/**
	 * @brief Current maven file being processed.
	 */
	string currentFile;
	
	/**
	 * @brief Current active namespace.
	 */
	string currentNamespace;
	
	/**
	 * @brief Current active class name (bare name).
	 */
	string currentClass;
	
	/**
	 * @brief Current function.
	 */
	string currentFunction;
	int currentBlockDepth;
	int dissectDepth;
	StringList reservedWords;
	vector<MavenVariables> localScope;
	vector<string> imports;
	string doc;
	vector<MavenBrackets> bracketStack;
	vector<MavenObjectDiscovery> discovery;
	
	/**
	 * @brief The total number of lines parsed.
	 */
	int totalLines;
	
	/**
	 * @brief The total number of files parsed.
	 */
	int totalFiles;
	
	long start;
	
	/**
	 * @brief Objects to check for ObjectNilException and BoundsException.
	 */
	vector<MavenPair> objectCheck;
	
	vector<string> postLines;
	
	vector<string> selectors;
	
	/**
	 * @brief This is collected throughout the dissection of the line then prepended before flushing
	 *        to the CPP file.
	 */
	string beforeLine;
	
	/**
	 * @brief The current directory is where maven is executed from, not where maven is located.
	 */
	string currentDirectory;
	
	/**
	 * @brief The location of the bin/ directory that the maven executable resides in.
	 */
	string binDirectory;
	
	/**
	 * @brief Filled with maven.ini before each maven compile.
	 */
	MavenINI iniFile;
	
	/**
	 * @brief All the extra .o's to include in the compilation.
	 */
	StringList extraLibraries;
	
	/**
	 * @brief Set and reset with switch().
	 */
	string switchArgument;
	
	/**
	 * @brief The UID is a unique ID for naming temporary objects.
	 */
	int UID;
	
	/**
	 * @brief For use with the 'with' statement.
	 */
	MavenVariable withObject;
	
	/**
	 * @brief The last statements, important for 'if' and 'do'.
	 */
	string lastStatament;
	
public:
	
	/**
	 * @brief Compile object file but do not link.
	 */
	bool option_c;
	
	/**
	 * @brief Execute after compilation.
	 */
	bool option_e;
	
	/**
	 * @brief Show help and quit.
	 */
	bool option_h;
	
	/**
	 * @brief Input file(s)
	 */
	string option_i;
	
	/**
	 * @brief Keep CPP and MMAP
	 */
	bool option_K;
	
	/**
	 * @brief Result executable/object name.
	 */
	string option_n;
	
	/**
	 * @brief Optimisation level (GNU specific.)
	 */
	string option_o;
	
	/**
	 * @brief Show compilation statistics.
	 */
	bool option_r;
	
	/**
	 * @brief Object-safe level.
	 */
	int option_s;
	
	/**
	 * @brief Print version and quit.
	 */
	bool option_v;
	
	/**
	 * @brief Show all compiler variables before compiling.
	 */
	bool option_V;
	
	/**
	 * @brief Supress specific warning.
	 */
	string option_w;
	
	/**
	 * @brief Show specific warning.
	 */
	string option_W;
	
	/**
	 * @brief Do not compile or link (no input file required.)
	 */
	bool option_x;
	
	/**
	 * @brief Place for SQL documentation.
	 */
	string option_doc_sql;
	
	/**
	 * @brief Place for SQLite3 documentation.
	 */
	string option_doc_sqlite3;
	
	/**
	 * @brief Place for XML documentation.
	 */
	string option_doc_xml;
	
	/**
	 * @brief Place for HTML documentation.
	 */
	string option_doc_html;
	
	/**
	 * @brief Where maven was originally executed from.
	 */
	string option_base_path;
	
public:
	
	MavenCompiler();
	
};

#endif
