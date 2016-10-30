#pragma once

#include <panda/core.h>

namespace graphview
{

namespace graphics
{

	struct PANDA_CORE_API DrawColors
	{
		// Can be modified to pass current object info
		unsigned int penColor = 0;
		unsigned int fillColor = 0;
		float penWidth = 1.0f;

		// Other colors
		unsigned int midLightColor = 0;
		unsigned int lightColor = 0;
		unsigned int highlightColor = 0;
		unsigned int backgroundColor = 0;
	};

} // namespace graphics

} // namespace graphview
