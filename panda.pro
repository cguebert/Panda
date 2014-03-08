TEMPLATE      = app
HEADERS       = \
	modules/particles/Particle.h \
	modules/particles/ParticleEngine.h \
	modules/particles/ParticleEffector.h \
	panda/BaseClass.h \
	panda/BaseData.h \
	panda/Data.h \
	panda/DataAccessor.h \
	panda/DataCopier.h \
	panda/DataFactory.h \
	panda/DataNode.h \
	panda/Dockable.h \
	panda/GenericObject.h \
	panda/Group.h \
	panda/Layer.h \
	panda/ObjectFactory.h \
	panda/PandaDocument.h \
	panda/PandaObject.h \
	panda/Renderer.h \
	panda/helper/Algorithm.h \
	panda/helper/Factory.h \
	panda/helper/GradientCache.h \
	panda/helper/Point.h \
	panda/helper/PointsGrid.h \
	panda/helper/Random.h \
	panda/helper/UpdateLogger.h \
	panda/types/Animation.h \
	panda/types/AnimationTraits.h \
	panda/types/DataTraits.h \
	panda/types/DataTypeId.h \
	panda/types/Gradient.h \
	panda/types/Path.h \
	panda/types/Topology.h \
	panda/types/TypeConverter.h \
	ui/Annotation.h \
	ui/DatasTable.h \
	ui/EditGroupDialog.h \
	ui/GraphView.h \
	ui/LayersTab.h \
	ui/ListObjectsAndTypes.h \
	ui/MainWindow.h \
	ui/OpenGLRenderView.h \
	ui/QuickCreateDialog.h \
	ui/RenderView.h \
	ui/UpdateLoggerDialog.h \
	ui/drawstruct/AnnotationDrawStruct.h \
	ui/drawstruct/DockableDrawStruct.h \
	ui/drawstruct/GenericObjectDrawStruct.h \
	ui/drawstruct/GroupObjectDrawStruct.h \
	ui/drawstruct/ObjectDrawStruct.h \
	ui/widget/AnimationDataWidgetDialog.h \
	ui/widget/DataWidget.h \
	ui/widget/DataWidgetFactory.h \
	ui/widget/EditGradientDialog.h \
	ui/widget/ListDataWidgetDialog.h \
	ui/widget/OpenDialogDataWidget.h \
	ui/widget/SimpleDataWidget.h \
	ui/widget/StructTraits.h \
	ui/widget/TableDataWidgetDialog.h
SOURCES       = main.cpp \
	modules/Replicator.cpp \
	modules/animation/AnimLength.cpp \
	modules/animation/AnimValue.cpp \
	modules/animation/Interpolation.cpp \
	modules/animation/List2Anim.cpp \
	modules/generators/UserValue.cpp \
	modules/generators/color/ComposeColors.cpp \
	modules/generators/color/RandomColors.cpp \
	modules/generators/gradient/Anim2Gradient.cpp \
	modules/generators/gradient/SimpleGradients.cpp \
	modules/generators/integer/RandomSeed.cpp \
	modules/generators/image/LoadImage.cpp \
	modules/generators/point/ComposePoints.cpp \
	modules/generators/point/MouseInfo.cpp \
	modules/generators/point/PoissonDistribution.cpp \
	modules/generators/point/RandomPoints.cpp \
	modules/generators/point/RenderSize.cpp \
	modules/generators/real/AnimationTime.cpp \
	modules/generators/real/Enumeration.cpp \
	modules/generators/real/RandomNumbers.cpp \
	modules/generators/rectangle/ComposeRectangles.cpp \
	modules/generators/text/DateTime.cpp \
	modules/generators/text/Format.cpp \
	modules/generators/topology/ComposeTopology.cpp \
	modules/generators/topology/Delaunay.cpp \
	modules/generators/topology/Grid.cpp \
	modules/generators/topology/MergeTopologies.cpp \
	modules/generators/topology/Voronoi.cpp \
	modules/list/Concatenation.cpp \
	modules/list/Condition.cpp \
	modules/list/DeleteItems.cpp \
	modules/list/ExtractHead.cpp \
	modules/list/ExtractTail.cpp \
	modules/list/FillList.cpp \
	modules/list/FilterItems.cpp \
	modules/list/FindItem.cpp \
	modules/list/ListBuffer.cpp \
	modules/list/ListItem.cpp \
	modules/list/ListReverse.cpp \
	modules/list/ListSize.cpp \
	modules/list/ReplaceItem.cpp \
	modules/list/RemoveDuplicates.cpp \
	modules/list/SortList.cpp \
	modules/math/image/ImageSize.cpp \
	modules/math/path/BezierSpline.cpp \
	modules/math/path/CubicBSpline.cpp \
	modules/math/path/Extrude.cpp \
	modules/math/path/PathMath.cpp \
	modules/math/point/NearestPoint.cpp \
	modules/math/point/PointListMath.cpp \
	modules/math/point/PointMath.cpp \
	modules/math/real/BooleanMath.cpp \
	modules/math/real/DoubleComputation.cpp \
	modules/math/real/DoubleListMath.cpp \
	modules/math/real/DoubleMath.cpp \
	modules/math/topology/Centroids.cpp \
	modules/math/topology/TopologyInfo.cpp \
	modules/modifiers/color/ColorInGradient.cpp \
	modules/modifiers/image/ColorInImage.cpp \
	modules/modifiers/image/ExtractImage.cpp \
	modules/modifiers/image/SaveImage.cpp \
	modules/modifiers/point/ConnectSegments.cpp \
	modules/modifiers/point/PointsInShapes.cpp \
	modules/modifiers/point/PointsUnion.cpp \
	modules/modifiers/point/RandomDisplacement.cpp \
	modules/modifiers/topology/FindPolygon.cpp \
	modules/modifiers/topology/Neighbors.cpp \
	modules/modifiers/topology/Relaxation.cpp \
	modules/particles/ParticleCreation.cpp \
	modules/particles/ParticleDestruction.cpp \
	modules/particles/ParticleEngine.cpp \
	modules/particles/ParticleForceField.cpp \
	modules/render/RenderCircle.cpp \
	modules/render/RenderDisk.cpp \
	modules/render/RenderGradient.cpp \
	modules/render/RenderImage.cpp \
	modules/render/RenderLine.cpp \
	modules/render/RenderMesh.cpp \
	modules/render/RenderPolygon.cpp \
	modules/render/RenderRect.cpp \
	modules/render/RenderText.cpp \
	panda/BaseClass.cpp \
	panda/BaseData.cpp \
	panda/Data.cpp \
	panda/Data.inl \
	panda/DataFactory.cpp \
	panda/DataNode.cpp \
	panda/Dockable.cpp \
	panda/GenericObject.cpp \
	panda/Group.cpp \
	panda/Layer.cpp \
	panda/ObjectFactory.cpp \
	panda/PandaDocument.cpp \
	panda/PandaObject.cpp \
	panda/Renderer.cpp \
	panda/helper/Factory.cpp \
	panda/helper/GradientCache.cpp \
	panda/helper/Point.cpp \
	panda/helper/PointsGrid.cpp \
	panda/helper/Random.cpp \
	panda/helper/UpdateLogger.cpp \
	panda/types/Animation.cpp \
	panda/types/Animation.inl \
	panda/types/DataTypeId.cpp \
	panda/types/Gradient.cpp \
	panda/types/Path.cpp \
	panda/types/Topology.cpp \
	panda/types/TypeConverter.cpp \
	ui/Annotation.cpp \
	ui/DatasTable.cpp \
	ui/EditGroupDialog.cpp \
	ui/GraphView.cpp \
	ui/LayersTab.cpp \
	ui/ListObjectsAndTypes.cpp \
	ui/MainWindow.cpp \
	ui/OpenGLRenderView.cpp \
	ui/QuickCreateDialog.cpp \
	ui/RenderView.cpp \
	ui/UpdateLoggerDialog.cpp \
	ui/drawstruct/AnnotationDrawStruct.cpp \
	ui/drawstruct/DockableDrawStruct.cpp \
	ui/drawstruct/GenericObjectDrawStruct.cpp \
	ui/drawstruct/GroupObjectDrawStruct.cpp \
	ui/drawstruct/ObjectDrawStruct.cpp \
	ui/widget/ColorDataWidget.cpp \
	ui/widget/EditGradientDialog.cpp \
	ui/widget/DataWidgetFactory.cpp \
	ui/widget/ListDataWidgetDialog.cpp \
	ui/widget/NumericalDataWidget.cpp \
	ui/widget/PathDataWidget.cpp \
	ui/widget/StringDataWidget.cpp \
	ui/widget/TableDataWidgetDialog.cpp \
	ui/widget/AnimationDataWidgetDialog.cpp
RESOURCES     = panda.qrc
win32:RC_FILE = panda.rc
INCLUDEPATH = .
QT += widgets xml opengl
CONFIG += c++11
DEFINES +=	PANDA_LOG_EVENTS \
			_CRT_SECURE_NO_WARNINGS \
			_SCL_SECURE_NO_WARNINGS

BOOST="C:\Users\Christophe\Documents\Programmation\boost_1_55_0"

INCLUDEPATH += $${BOOST}/
LIBPATH += $${BOOST}/stage/lib
