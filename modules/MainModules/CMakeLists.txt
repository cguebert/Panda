cmake_minimum_required(VERSION 3.1)

set(PROJECT_NAME "MainModules")
project(${PROJECT_NAME})

set(HEADER_FILES
)

set(SOURCE_FILES
	mainModule.cpp
	Replicator.cpp
	animation/AnimLength.cpp
	animation/AnimValue.cpp
	animation/Interpolation.cpp
	animation/List2Anim.cpp
	generators/color/ComposeColors.cpp
	generators/color/RandomColors.cpp
	generators/gradient/Anim2Gradient.cpp
	generators/gradient/ComposeGradient.cpp
	generators/gradient/GradientTexture.cpp
	generators/gradient/SimpleGradients.cpp
	generators/integer/RandomSeed.cpp
	generators/image/CreateTexture.cpp
	generators/image/DocumentImage.cpp
	generators/image/LoadImage.cpp
	generators/point/ComposePoints.cpp
	generators/point/MouseInfo.cpp
	generators/point/Noise.cpp
	generators/point/PoissonDistribution.cpp
	generators/point/RandomPoints.cpp
	generators/point/RenderSize.cpp
	generators/real/AnimationTime.cpp
	generators/real/Enumeration.cpp
	generators/real/RandomNumbers.cpp
	generators/rectangle/ComposeRectangles.cpp
	generators/text/DateTime.cpp
	generators/text/Format.cpp
	generators/shader/SetShaderValues.cpp
	math/image/ImageSize.cpp
	math/point/NearestPoint.cpp
	math/point/PointListMath.cpp
	math/point/PointMath.cpp
	math/real/BooleanMath.cpp
	math/real/DoubleComputation.cpp
	math/real/DoubleListMath.cpp
	math/real/DoubleMath.cpp
	math/rect/RectangleMath.cpp
	modifiers/color/BlendColors.cpp
	modifiers/color/ColorInGradient.cpp
	modifiers/image/ColorInImage.cpp
	modifiers/image/ExtractImage.cpp
	modifiers/image/SaveImage.cpp
	modifiers/point/PointsInShapes.cpp
	modifiers/point/PointsUnion.cpp
	modifiers/point/RandomDisplacement.cpp
)

GroupFiles(HEADER_FILES)
GroupFiles(SOURCE_FILES)

EnablePrecompiledHeaders(SOURCE_FILES)

add_library(${PROJECT_NAME} SHARED ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Modules")

target_link_libraries(${PROJECT_NAME} "PandaCore")

find_package(Qt5Widgets)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION ${MODULES_DIR})

add_definitions(-DTARGET="${PROJECT_NAME}")
