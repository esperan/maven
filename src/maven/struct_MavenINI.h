/*
 *  maven Programming Language
 *  Created by Elliot Chance <elliot@chancemedia.com>.
 */

#ifndef _MAVEN_STRUCT_INI_H
#define _MAVEN_STRUCT_INI_H 1

#include "maven.h"
#include "struct_MavenINIPair.h"

class MavenINI {
protected:
	vector<MavenINIPair> ini;
	
public:
	MavenINI();
	bool setKey(string key, string value);
	string getKey(string key);
};

#endif
