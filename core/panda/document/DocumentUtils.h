#pragma once

#include <panda/core.h>

namespace panda 
{

	class BaseData;
	class PandaDocument;
	class ObjectsList;

	// Create a user value object based on the given data, and copy its value
	void PANDA_CORE_API copyDataToUserValue(const BaseData* data, PandaDocument* document, ObjectsList& objectList);

} // namespace panda

