TEMPLATE = lib
CONFIG += dll c++11
TARGET = PandaModules
HEADERS       = \
	generators/UserValue.h \
	particles/Particle.h \
	particles/ParticleEngine.h \
	particles/ParticleEffector.h
SOURCES       = \
	mainModule.cpp \
	Replicator.cpp \
	animation/AnimLength.cpp \
	animation/AnimValue.cpp \
	animation/Interpolation.cpp \
	animation/List2Anim.cpp \
	generators/UserValue.cpp \
	generators/color/ComposeColors.cpp \
	generators/color/RandomColors.cpp \
	generators/gradient/Anim2Gradient.cpp \
	generators/gradient/ComposeGradient.cpp \
	generators/gradient/GradientTexture.cpp \
	generators/gradient/SimpleGradients.cpp \
	generators/integer/RandomSeed.cpp \
	generators/image/CreateTexture.cpp \
	generators/image/DocumentImage.cpp \
	generators/image/LoadImage.cpp \
	generators/point/ComposePoints.cpp \
	generators/point/MouseInfo.cpp \
	generators/point/Noise.cpp \
	generators/point/PoissonDistribution.cpp \
	generators/point/RandomPoints.cpp \
	generators/point/RenderSize.cpp \
	generators/real/AnimationTime.cpp \
	generators/real/Enumeration.cpp \
	generators/real/RandomNumbers.cpp \
	generators/rectangle/ComposeRectangles.cpp \
	generators/text/DateTime.cpp \
	generators/text/Format.cpp \
	generators/shader/SetShaderValues.cpp \
	generators/mesh/ComposeMesh.cpp \
	generators/mesh/Delaunay.cpp \
	generators/mesh/Grid.cpp \
	generators/mesh/MergeMeshes.cpp \
	generators/mesh/Voronoi.cpp \
	list/Concatenation.cpp \
	list/Condition.cpp \
	list/DeleteItems.cpp \
	list/ExtractHead.cpp \
	list/ExtractTail.cpp \
	list/FillList.cpp \
	list/FilterItems.cpp \
	list/FindItem.cpp \
	list/InterleaveSequence.cpp \
	list/ListBuffer.cpp \
	list/ListItem.cpp \
	list/ListReverse.cpp \
	list/ListSize.cpp \
	list/ReplaceItem.cpp \
	list/RemoveDuplicates.cpp \
	list/SortList.cpp \
	math/image/ImageSize.cpp \
	math/path/BezierSpline.cpp \
	math/path/CubicBSpline.cpp \
	math/path/Extrude.cpp \
	math/path/PathMath.cpp \
	math/point/NearestPoint.cpp \
	math/point/PointListMath.cpp \
	math/point/PointMath.cpp \
	math/real/BooleanMath.cpp \
	math/real/DoubleComputation.cpp \
	math/real/DoubleListMath.cpp \
	math/real/DoubleMath.cpp \
	math/rect/RectangleMath.cpp \
	math/mesh/Centroids.cpp \
	math/mesh/MeshInfo.cpp \
	modifiers/color/BlendColors.cpp \
	modifiers/color/ColorInGradient.cpp \
	modifiers/image/ColorInImage.cpp \
	modifiers/image/ExtractImage.cpp \
	modifiers/image/MergeImages.cpp \
	modifiers/image/SaveImage.cpp \
	modifiers/image/ShaderEffects.cpp \
	modifiers/point/ConnectSegments.cpp \
	modifiers/point/PointsInShapes.cpp \
	modifiers/point/PointsUnion.cpp \
	modifiers/point/RandomDisplacement.cpp \
	modifiers/mesh/FindTriangle.cpp \
	modifiers/mesh/Neighbors.cpp \
	modifiers/mesh/Relaxation.cpp \
	particles/ParticleCollision.cpp \
	particles/ParticleCreation.cpp \
	particles/ParticleDestruction.cpp \
	particles/ParticleEngine.cpp \
	particles/ParticleForceField.cpp \
	render/RenderCircle.cpp \
	render/RenderDisk.cpp \
	render/RenderImage.cpp \
	render/RenderFill.cpp \
	render/RenderLine.cpp \
	render/RenderMesh.cpp \
	render/RenderRect.cpp \
	render/RenderSprite.cpp \
	render/RenderText.cpp \
	render/RenderTriangle.cpp \
	render/modifiers/RenderModifierBlend.cpp \
	render/modifiers/RenderModifierTransformations.cpp
INCLUDEPATH = .
QT += widgets xml opengl
CONFIG += c++11
DEFINES +=	PANDA_LOG_EVENTS \
			_CRT_SECURE_NO_WARNINGS \
			_SCL_SECURE_NO_WARNINGS

Release:DESTDIR = ../bin/modules
Debug:DESTDIR = ../bin/modules_d
Release:LIBS += ../bin/PandaCore.lib
Debug:LIBS += ../bin/PandaCore_d.lib

PRECOMPILED_HEADER = ../core/panda/pch.h

#DEFINES += PANDA_DOUBLE

BOOST="../../boost_1_56_0"

INCLUDEPATH += $${BOOST}/
INCLUDEPATH += ".."
INCLUDEPATH += "../core"
LIBPATH += $${BOOST}/stage/lib
