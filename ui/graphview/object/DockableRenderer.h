#ifndef DOCKABLEDRAWSTRUCT_H
#define DOCKABLEDRAWSTRUCT_H

#include <panda/object/Dockable.h>

#include <ui/graphview/object/ObjectRenderer.h>

class DockObjectRenderer : public ObjectRenderer
{
public:
	DockObjectRenderer(GraphView* view, panda::DockObject* m_object);

	void placeDockableObjects(bool forceMove = false);

	panda::types::Point getObjectSize() override;
	panda::types::Rect getTextArea() override;
	void createShape() override;

	int getDockableIndex(const panda::types::Rect& rect);

	static const int dockEmptyRendererHeight = 30;
	static const int dockHoleWidth = 80;
	static const int dockHoleMargin = 3;
	static const int dockRendererMargin = 10;
protected:
	panda::DockObject* m_dockObject;
	std::vector<int> m_dockablesY;	// Used only in "getDockableIndex"
};

class DockableObjectRenderer : public ObjectRenderer
{
public:
	DockableObjectRenderer(GraphView* view, panda::DockableObject* m_object);

	void update() override;
	void createShape() override;
	bool contains(const panda::types::Point& point) override;
	panda::types::Point getObjectSize() override;
	panda::types::Rect getTextArea() override;

	static const int dockableCircleWidth = 20;

	static const int dockableWithOutputAdds = 25;
	static const int dockableWithOutputRect = 15; // How much rectangle area we add
	static const int dockableWithOutputArc = 15;

protected:
	bool m_hasOutputs = false;
};

#endif
