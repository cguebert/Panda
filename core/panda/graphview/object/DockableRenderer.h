#pragma once

#include <panda/object/Dockable.h>

#include <panda/graphview/object/ObjectRenderer.h>

namespace panda
{

namespace graphview
{

namespace object
{

class DockObjectRenderer : public ObjectRenderer
{
public:
	DockObjectRenderer(GraphView* view, DockObject* m_object);

	void placeDockableObjects(bool forceMove = false);

	types::Point getObjectSize() override;
	types::Rect getTextArea() override;
	void createShape() override;

	int getDockableIndex(const types::Rect& rect);

	static const int dockEmptyRendererHeight = 30;
	static const int dockHoleWidth = 80;
	static const int dockHoleMargin = 3;
	static const int dockRendererMargin = 10;
protected:
	DockObject* m_dockObject;
	std::vector<float> m_dockablesY;	// Used only in "getDockableIndex"
};

class DockableObjectRenderer : public ObjectRenderer
{
public:
	DockableObjectRenderer(GraphView* view, DockableObject* m_object);

	void update() override;
	void createShape() override;
	bool contains(const types::Point& point) override;
	types::Point getObjectSize() override;
	types::Rect getTextArea() override;

	static const int dockableCircleWidth = 20;

	static const int dockableWithOutputAdds = 25;
	static const int dockableWithOutputRect = 15; // How much rectangle area we add
	static const int dockableWithOutputArc = 15;

protected:
	bool m_hasOutputs = false;
};

} // namespace object

} // namespace graphview

} // namespace panda
