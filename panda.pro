TEMPLATE      = app
HEADERS       = \
	modules/generators/UserValue.h \
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
	panda/Scheduler.h \
	panda/helper/Algorithm.h \
	panda/helper/Factory.h \
	panda/helper/Gl.h \
	panda/helper/GradientCache.h \
	panda/helper/Perlin.h \
	panda/helper/PointsGrid.h \
	panda/helper/Polygon.h \
	panda/helper/Random.h \
	panda/helper/ShaderCache.h \
	panda/helper/UpdateLogger.h \
	panda/helper/system/Config.h \
	panda/types/Animation.h \
	panda/types/AnimationTraits.h \
	panda/types/Color.h \
	panda/types/DataTraits.h \
	panda/types/DataTypeId.h \
	panda/types/Gradient.h \
	panda/types/ImageWrapper.h \
	panda/types/Mesh.h \
	panda/types/Path.h \
	panda/types/Point.h \
	panda/types/Rect.h \
	panda/types/Shader.h \
	panda/types/TypeConverter.h \
	ui/Annotation.h \
	ui/DatasTable.h \
	ui/EditGroupDialog.h \
	ui/GraphView.h \
	ui/GroupsManager.h \
	ui/LayersTab.h \
	ui/LinkTag.h \
	ui/ListObjectsAndTypes.h \
	ui/MainWindow.h \
	ui/OpenGLRenderView.h \
	ui/QuickCreateDialog.h \
	ui/UpdateLoggerDialog.h \
	ui/command/AddObjectCommand.h \
	ui/command/CommandId.h \
	ui/command/CreateGroupCommand.h \
	ui/command/DeleteObjectCommand.h \
	ui/command/DetachDockableCommand.h \
	ui/command/LinkDatasCommand.h \
	ui/command/ModifyAnnotationCommand.h \
	ui/command/MoveObjectCommand.h \
	ui/command/RemoveGenericDataCommand.h \
	ui/command/SetDataValueCommand.h \
	ui/drawstruct/AnnotationDrawStruct.h \
	ui/drawstruct/DockableDrawStruct.h \
	ui/drawstruct/GenericObjectDrawStruct.h \
	ui/drawstruct/GroupObjectDrawStruct.h \
	ui/drawstruct/ObjectDrawStruct.h \
	ui/widget/AnimationDataWidgetDialog.h \
	ui/widget/DataWidget.h \
	ui/widget/DataWidgetFactory.h \
	ui/widget/EditGradientDialog.h \
	ui/widget/EditMeshDialog.h \
	ui/widget/EditShaderDialog.h \
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
	modules/generators/gradient/ComposeGradient.cpp \
	modules/generators/gradient/SimpleGradients.cpp \
	modules/generators/integer/RandomSeed.cpp \
	modules/generators/image/CreateTexture.cpp \
	modules/generators/image/DocumentImage.cpp \
	modules/generators/image/LoadImage.cpp \
	modules/generators/point/ComposePoints.cpp \
	modules/generators/point/MouseInfo.cpp \
	modules/generators/point/Noise.cpp \
	modules/generators/point/PoissonDistribution.cpp \
	modules/generators/point/RandomPoints.cpp \
	modules/generators/point/RenderSize.cpp \
	modules/generators/real/AnimationTime.cpp \
	modules/generators/real/Enumeration.cpp \
	modules/generators/real/RandomNumbers.cpp \
	modules/generators/rectangle/ComposeRectangles.cpp \
	modules/generators/text/DateTime.cpp \
	modules/generators/text/Format.cpp \
	modules/generators/shader/SetShaderValues.cpp \
	modules/generators/mesh/ComposeMesh.cpp \
	modules/generators/mesh/Delaunay.cpp \
	modules/generators/mesh/Grid.cpp \
	modules/generators/mesh/MergeMeshes.cpp \
	modules/generators/mesh/Voronoi.cpp \
	modules/list/Concatenation.cpp \
	modules/list/Condition.cpp \
	modules/list/DeleteItems.cpp \
	modules/list/ExtractHead.cpp \
	modules/list/ExtractTail.cpp \
	modules/list/FillList.cpp \
	modules/list/FilterItems.cpp \
	modules/list/FindItem.cpp \
	modules/list/InterleaveSequence.cpp \
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
	modules/math/rect/RectangleMath.cpp \
	modules/math/mesh/Centroids.cpp \
	modules/math/mesh/MeshInfo.cpp \
	modules/modifiers/color/BlendColors.cpp \
	modules/modifiers/color/ColorInGradient.cpp \
	modules/modifiers/image/ColorInImage.cpp \
	modules/modifiers/image/ExtractImage.cpp \
	modules/modifiers/image/SaveImage.cpp \
	modules/modifiers/point/ConnectSegments.cpp \
	modules/modifiers/point/PointsInShapes.cpp \
	modules/modifiers/point/PointsUnion.cpp \
	modules/modifiers/point/RandomDisplacement.cpp \
	modules/modifiers/mesh/FindTriangle.cpp \
	modules/modifiers/mesh/Neighbors.cpp \
	modules/modifiers/mesh/Relaxation.cpp \
	modules/particles/ParticleCollision.cpp \
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
	modules/render/RenderRect.cpp \
	modules/render/RenderSprite.cpp \
	modules/render/RenderText.cpp \
	modules/render/RenderTriangle.cpp \
	modules/render/modifiers/RenderModifierBlend.cpp \
	modules/render/modifiers/RenderModifierTransformations.cpp \
	panda/BaseClass.cpp \
	panda/BaseData.cpp \
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
	panda/Scheduler.cpp \
	panda/helper/Factory.cpp \
	panda/helper/GradientCache.cpp \
	panda/helper/Perlin.cpp \
	panda/helper/PointsGrid.cpp \
	panda/helper/Polygon.cpp \
	panda/helper/Random.cpp \
	panda/helper/ShaderCache.cpp \
	panda/helper/UpdateLogger.cpp \
	panda/types/Animation.inl \
	panda/types/Color.cpp \
	panda/types/DataTraits.cpp \
	panda/types/DataTypeId.cpp \
	panda/types/Gradient.cpp \
	panda/types/ImageWrapper.cpp \
	panda/types/Mesh.cpp \
	panda/types/Path.cpp \
	panda/types/Point.cpp \
	panda/types/Rect.cpp \
	panda/types/Shader.cpp \
	panda/types/StandardTypes.cpp \
	panda/types/TypeConverter.cpp \
	ui/Annotation.cpp \
	ui/DatasTable.cpp \
	ui/EditGroupDialog.cpp \
	ui/GraphView.cpp \
	ui/GroupsManager.cpp \
	ui/LayersTab.cpp \
	ui/LinkTag.cpp \
	ui/ListObjectsAndTypes.cpp \
	ui/MainWindow.cpp \
	ui/OpenGLRenderView.cpp \
	ui/QuickCreateDialog.cpp \
	ui/UpdateLoggerDialog.cpp \
	ui/command/AddObjectCommand.cpp \
	ui/command/CommandId.cpp \
	ui/command/CreateGroupCommand.cpp \
	ui/command/DeleteObjectCommand.cpp \
	ui/command/DetachDockableCommand.cpp \
	ui/command/LinkDatasCommand.cpp \
	ui/command/ModifyAnnotationCommand.cpp \
	ui/command/MoveObjectCommand.cpp \
	ui/command/RemoveGenericDataCommand.cpp \
	ui/drawstruct/AnnotationDrawStruct.cpp \
	ui/drawstruct/DockableDrawStruct.cpp \
	ui/drawstruct/GenericObjectDrawStruct.cpp \
	ui/drawstruct/GroupObjectDrawStruct.cpp \
	ui/drawstruct/ObjectDrawStruct.cpp \
	ui/widget/ColorDataWidget.cpp \
	ui/widget/EditGradientDialog.cpp \
	ui/widget/EditMeshDialog.cpp \
	ui/widget/EditShaderDialog.cpp \
	ui/widget/DataWidgetFactory.cpp \
	ui/widget/NumericalDataWidget.cpp \
	ui/widget/PointDataWidget.cpp \
	ui/widget/RectDataWidget.cpp \
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
#DEFINES += PANDA_DOUBLE

BOOST="C:\Users\Christophe\Documents\Programmation\boost_1_55_0"

INCLUDEPATH += $${BOOST}/
LIBPATH += $${BOOST}/stage/lib
