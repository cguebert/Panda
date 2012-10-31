TEMPLATE      = app
HEADERS       = \
	helper/PointsGrid.h \
	helper/Random.h \
	ui/RenderView.h \
	ui/MainWindow.h \
	ui/GraphView.h \
	ui/DatasTable.h \
	panda/Renderer.h \
	panda/PandaObject.h \
	panda/PandaDocument.h \
	panda/ObjectFactory.h \
	panda/Layer.h \
	panda/DataNode.h \
	panda/Data.h \
	panda/BaseData.h \
	panda/Animation.h \
	ui/DatasEditDialog.h \
	panda/Group.h \
	ui/ObjectDrawStruct.h \
	panda/Dockable.h \
	panda/GenericObject.h
SOURCES       = main.cpp \
	helper/PointsGrid.cpp \
	helper/Random.cpp \
	ui/RenderView.cpp \
	ui/MainWindow.cpp \
	ui/GraphView.cpp \
	ui/DatasTable.cpp \
	panda/Renderer.cpp \
	panda/PandaObject.cpp \
	panda/PandaDocument.cpp \
	panda/ObjectFactory.cpp \
	panda/Layer.cpp \
	panda/DataNode.cpp \
	panda/Data.cpp \
	panda/BaseData.cpp \
	panda/Animation.cpp \
	modules/render/RenderDisk.cpp \
	modules/render/RenderRect.cpp \
	modules/Replicator.cpp \
	modules/animation/Interpolation.cpp \
	modules/render/RenderGradient.cpp \
	modules/animation/AnimValue.cpp \
	modules/animation/List2Anim.cpp \
	modules/generators/UserValue.cpp \
	modules/generators/gradient/SimpleGradients.cpp \
	modules/generators/point/RandomPoints.cpp \
	modules/generators/point/ComposePoints.cpp \
	modules/generators/real/RandomNumbers.cpp \
	modules/generators/real/AnimationTime.cpp \
	modules/generators/rectangle/ComposeRectangles.cpp \
	modules/generators/color/ComposeColors.cpp \
	modules/math/real/DoubleMath.cpp \
	modules/math/real/DoubleListMath.cpp \
	modules/math/point/PointListMath.cpp \
	modules/math/point/PointMath.cpp \
	modules/render/RenderLine.cpp \
	modules/generators/real/Enumeration.cpp \
	modules/generators/point/PoissonDistribution.cpp \
	modules/list/ListItem.cpp \
	modules/list/FillList.cpp \
	modules/generators/text/DateTime.cpp \
	modules/modifiers/point/PointsUnion.cpp \
	ui/DatasEditDialog.cpp \
	panda/Group.cpp \
	ui/ObjectDrawStruct.cpp \
	panda/Dockable.cpp \
	modules/list/ModifyList.cpp \
	modules/list/ListBuffer.cpp \
	panda/GenericObject.cpp \
    modules/generators/point/MouseInfo.cpp
RESOURCES     = panda.qrc
INCLUDEPATH = . \
	../boost_1_51_0
