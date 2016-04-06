#pragma once

#include <panda/types/Color.h>
#include <panda/types/FloatVector.h>
#include <panda/types/Gradient.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/IntVector.h>
#include <panda/types/Polygon.h>
#include <panda/types/Mesh.h>
#include <panda/types/Rect.h>
#include <panda/types/Shader.h>

#include <tuple>

namespace panda
{

typedef std::tuple<int, float, types::Color, types::Point, types::Rect, std::string, types::ImageWrapper,
	types::Gradient, types::Shader, types::Mesh, types::Path, types::Polygon, types::FloatVector, types::IntVector> allDataTypes;
typedef std::tuple<int, float, types::Color, types::Point, types::Rect, std::string, types::FloatVector, types::IntVector> allSearchableTypes;
typedef std::tuple<int, float, types::Color, types::Point, types::Rect, std::string> allSortableTypes;
typedef std::tuple<float, types::Color, types::Point, types::Gradient> allAnimationTypes;

} // namespace panda
