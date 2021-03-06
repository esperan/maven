/*
 *  maven Programming Language
 *  Created by Elliot Chance <elliot@chancemedia.com>.
 */

#include "maven.h"
#include "struct_MavenCompiler.h"
#include "struct_MavenObject.h"

int findVariableID(MavenCompiler* c, int namespaceID, int objectID, string variable) {
	if(namespaceID < 0 || objectID < 0)
		return -1;
	
	for(int i = 0; i < c->namespaces->at(namespaceID).objects->at(objectID)->variables->length(); ++i)
		if(c->namespaces->at(namespaceID).objects->at(objectID)->variables->at(i).name == variable)
			return i;
	
	// can't find it
	return -1;
}
