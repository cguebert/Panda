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
	panda/helper/PointsGrid.h \
	panda/helper/Random.h \
	panda/types/Animation.h \
	panda/types/AnimationTraits.h \
	panda/types/DataTraits.h \
	panda/types/DataTypeId.h \
	panda/types/Gradient.h \
	panda/types/Topology.h \
	ui/AnimationDataWidgetDialog.h \
	ui/DataWidget.h \
	ui/DatasTable.h \
	ui/DockableDrawStruct.h \
	ui/EditGradientDialog.h \
	ui/EditGroupDialog.h \
	ui/GenericObjectDrawStruct.h \
	ui/GraphView.h \
	ui/GroupObjectDrawStruct.h \
	ui/LayersTab.h \
	ui/ListDataWidgetDialog.h \
	ui/MainWindow.h \
	ui/ObjectDrawStruct.h \
	ui/QuickCreateDialog.h \
	ui/RenderView.h \
	ui/SimpleDataWidget.h \
	ui/StructTraits.h \
	ui/TableDataWidgetDialog.h \
    modules/generators/integer/RandomSeed.h
SOURCES       = main.cpp \
	modules/Replicator.cpp \
	modules/animation/AnimLength.cpp \
	modules/animation/AnimValue.cpp \
	modules/animation/Interpolation.cpp \
	modules/animation/List2Anim.cpp \
	modules/generators/UserValue.cpp \
	modules/generators/color/ColorInGradient.cpp \
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
	modules/generators/topology/Voronoi.cpp \
	modules/list/Concatenation.cpp \
	modules/list/Condition.cpp \
	modules/list/DeleteItems.cpp \
	modules/list/ExtractHead.cpp \
	modules/list/ExtractTail.cpp \
	modules/list/FillList.cpp \
	modules/list/FindItem.cpp \
	modules/list/ListBuffer.cpp \
	modules/list/ListItem.cpp \
	modules/list/ListReverse.cpp \
	modules/list/ListSize.cpp \
	modules/list/ReplaceItem.cpp \
	modules/math/image/ImageSize.cpp \
	modules/math/point/BezierSpline.cpp \
	modules/math/point/NearestPoint.cpp \
	modules/math/point/PointListMath.cpp \
	modules/math/point/PointMath.cpp \
	modules/math/real/DoubleComputation.cpp \
	modules/math/real/DoubleListMath.cpp \
	modules/math/real/DoubleMath.cpp \
	modules/math/topology/TopologyInfo.cpp \
	modules/modifiers/image/ColorInImage.cpp \
	modules/modifiers/image/ExtractImage.cpp \
	modules/modifiers/image/SaveImage.cpp \
	modules/modifiers/point/PointsInShapes.cpp \
	modules/modifiers/point/PointsUnion.cpp \
	modules/modifiers/point/RandomDisplacement.cpp \
	modules/particles/ParticleCreation.cpp \
	modules/particles/ParticleDestruction.cpp \
	modules/particles/ParticleEngine.cpp \
	modules/particles/ParticleForceField.cpp \
	modules/render/RenderCircle.cpp \
	modules/render/RenderDisk.cpp \
	modules/render/RenderGradient.cpp \
	modules/render/RenderImage.cpp \
	modules/render/RenderLine.cpp \
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
	panda/helper/PointsGrid.cpp \
	panda/helper/Random.cpp \
	panda/types/Animation.cpp \
	panda/types/Animation.inl \
	panda/types/DataTypeId.cpp \
	panda/types/Gradient.cpp \
	panda/types/Topology.cpp \
	ui/DatasTable.cpp \
	ui/DockableDrawStruct.cpp \
	ui/EditGradientDialog.cpp \
	ui/EditGroupDialog.cpp \
	ui/GenericObjectDrawStruct.cpp \
	ui/GraphView.cpp \
	ui/GroupObjectDrawStruct.cpp \
	ui/LayersTab.cpp \
	ui/ListDataWidgetDialog.cpp \
	ui/MainWindow.cpp \
	ui/ObjectDrawStruct.cpp \
	ui/QuickCreateDialog.cpp \
	ui/RenderView.cpp \
	ui/SimpleDataWidget.cpp \
	ui/TableDataWidgetDialog.cpp
RESOURCES     = panda.qrc
win32:RC_FILE = panda.rc
INCLUDEPATH = . \
    ../boost_1_53_0
QT += widgets xml
CONFIG += c++11
