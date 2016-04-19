#ifndef DOCKABLEDRAWSTRUCT_H
#define DOCKABLEDRAWSTRUCT_H

#include <panda/object/Dockable.h>

#include <ui/drawstruct/ObjectDrawStruct.h>

class DockObjectDrawStruct : public ObjectDrawStruct
{
public:
	DockObjectDrawStruct(GraphView* view, panda::DockObject* m_object);

	void placeDockableObjects();

	void drawShape(DrawList& list, DrawColors& colors) override;
	panda::types::Point getObjectSize() override;
	panda::types::Rect getTextArea() override;
	void move(const panda::types::Point& delta) override;
	void moveVisual(const panda::types::Point& delta) override;
	void update() override;
	bool contains(const panda::types::Point& point) override;

	int getDockableIndex(const panda::types::Rect& rect);

	static const int dockEmptyRendererHeight = 30;
	static const int dockHoleWidth = 80;
	static const int dockHoleMargin = 3;
	static const int dockRendererMargin = 10;
protected:
	panda::DockObject* m_dockObject;
	DrawPath m_shapePath;
	DrawMesh m_shapeMesh;
	std::vector<int> m_dockablesY;	// Used only in "getDockableIndex"
};

class DockableObjectDrawStruct : public ObjectDrawStruct
{
public:
	DockableObjectDrawStruct(GraphView* view, panda::DockableObject* m_object);

	void drawShape(DrawList& list, DrawColors& colors) override;
	void moveVisual(const panda::types::Point& delta) override;
	void update() override;
	bool contains(const panda::types::Point& point) override;
	panda::types::Point getObjectSize() override;
	panda::types::Rect getTextArea() override;

	static const int dockableCircleWidth = 20;

	static const int dockableWithOutputAdds = 25;
	static const int dockableWithOutputRect = 15; // How much rectangle area we add
	static const int dockableWithOutputArc = 15;

protected:
	DrawPath m_shapePath;
	DrawMesh m_shapeMesh;
	bool m_hasOutputs = false;
};

#endif
