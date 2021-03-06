#pragma once

#include <panda/graphview/graphics/DrawList.h>

#include <memory>
#include <vector>

namespace panda
{
	class PandaObject;
	class BaseData;
	class BaseClass;
	class XmlElement;

	namespace msg
	{ class Observer; }

namespace graphview
{

namespace graphics {
	struct DrawColors;
	class DrawList;
}

class GraphView;
class MouseEvent;
class ObjectPositionAddon;
class ViewRenderer;

namespace object
{

class PANDA_CORE_API ObjectRenderer
{
public:
	using DataRectPair = std::pair<BaseData*, types::Rect>;

	ObjectRenderer(GraphView* view, PandaObject* obj);
	virtual ~ObjectRenderer();

	void initializeRenderer(ViewRenderer& viewRenderer);

	virtual void drawBackground(graphics::DrawList& list, graphics::DrawColors& colors) {}	// Called first
	virtual void draw(graphics::DrawList& list, graphics::DrawColors& colors, bool selected = false); // "Normal" draw
	virtual void drawForeground(graphics::DrawList& list, graphics::DrawColors& colors) {}	// Called last

	virtual void update();										// Recompute the information about this object
	virtual void move(const types::Point& delta);		// Move the position of the object in the view
	virtual bool contains(const types::Point& point);	// Is this point inside of the shape of this object ? (which can be complex)

	virtual bool mousePressEvent(const MouseEvent&) { return false; }	// Return true to capture the mouse for this object
	virtual void mouseMoveEvent(const MouseEvent&) {}
	virtual void mouseReleaseEvent(const MouseEvent&) {}

	virtual bool acceptsMagneticSnap() const; // If this object is used for the magnetic snap when moving objects

	virtual types::Point getObjectSize();
	
	const std::vector<DataRectPair>& getDataRects() const;
	BaseData* getDataAtPos(const types::Point &pt, types::Point* center = nullptr) const;
	bool getDataRect(const BaseData* data, types::Rect& rect) const;

	types::Rect getVisualArea() const;
	types::Rect getSelectionArea() const;
	types::Point getPosition() const;
	PandaObject* const getObject() const;
	GraphView* const getParentView() const;

	static const int objectDefaultWidth = 100;
	static const int objectDefaultHeight = 50;

	static const int dataRectSize = 10;
	static const int dataRectMargin = 5;

	static const int objectCorner = 5;

	virtual int dataStartY();

	bool updateIfDirty(); // Returns true if it was dirty (and we did something), false if there was nothing to do
	void setDirty();

protected:
	// The next 3 functions are here if we want to replace only part of the normal draw
	virtual void drawShape(graphics::DrawList& list, graphics::DrawColors& colors);
	virtual void drawDatas(graphics::DrawList& list, graphics::DrawColors& colors);
	virtual void drawText(graphics::DrawList& list, graphics::DrawColors& colors);

	virtual void createShape(); // Modify m_outline & m_fillShape

	virtual types::Rect getTextArea(); // The area in which we can render text
	virtual std::string getLabel() const; // The text to draw

	void drawData(graphics::DrawList& list, graphics::DrawColors& colors, const BaseData* data, const types::Rect& area);
	
	PandaObject* m_object;
	types::Rect m_visualArea, m_selectionArea;
	std::vector<DataRectPair> m_datas;

	std::string m_currentLabel;
	std::unique_ptr<graphics::DrawList> m_textDrawList;
	graphics::DrawPath m_outline;
	graphics::DrawMesh m_fillShape;

	std::unique_ptr<msg::Observer> m_observer;

private:
	void positionChanged(types::Point newPos);

	GraphView* m_parentView;
	ObjectPositionAddon& m_positionAddon;
	types::Point m_position;
	bool m_dirty = true;
};

inline bool ObjectRenderer::acceptsMagneticSnap() const
{ return true; }

inline bool ObjectRenderer::contains(const types::Point& point)
{ return m_selectionArea.contains(point) && m_outline.contains(point); }

inline types::Rect ObjectRenderer::getVisualArea() const
{ return m_visualArea; }

inline types::Rect ObjectRenderer::getSelectionArea() const
{ return m_selectionArea; }

inline types::Point ObjectRenderer::getPosition() const
{ return m_position; }

inline PandaObject* const ObjectRenderer::getObject() const
{ return m_object; }

inline GraphView* const ObjectRenderer::getParentView() const
{ return m_parentView; }

inline int ObjectRenderer::dataStartY()
{ return dataRectMargin; }

inline const std::vector<ObjectRenderer::DataRectPair>& ObjectRenderer::getDataRects() const
{ return m_datas; }

} // namespace object

} // namespace graphview

} // namespace panda
