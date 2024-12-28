#pragma once

#include <string>
#include "transform.h"

class Object {
public:
	std::string name;
	Transform transform;
	
	Object(const std::string& name);

	virtual void renderInspector();
};