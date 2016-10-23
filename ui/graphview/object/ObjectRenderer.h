#pragma once

#include <ui/graphview/graphics/DrawList.h>

#include <memory>

class QMouseEvent;

namespace panda
{
	class PandaObject;
	class BaseData;
	class BaseClass;
	class XmlElement;

	namespace msg
	{ class Observer; }
}

namespace graphview
{

namespace graphics {
	struct DrawColors;
}

class GraphView;
class ObjectPositionAddon;

namespace object
{

class ObjectRenderer
{
public:
	using DataRectPair = std::pair<panda::BaseData*, panda::types::Rect>;

	ObjectRenderer(GraphView* view, panda::PandaObject* obj);
	virtual ~ObjectRenderer();

	virtual void drawBackground(graphics::DrawList& list, graphics::DrawColors& colors) {}	// Called first
	virtual void draw(graphics::DrawList& list, graphics::DrawColors& colors, bool selected = false); // "Normal" draw
	virtual void drawForeground(graphics::DrawList& list, graphics::DrawColors& colors) {}	// Called last

	virtual void update();										// Recompute the information about this object
	virtual void move(const panda::types::Point& delta);		// Move the position of the object in the view
	virtual bool contains(const panda::types::Point& point);	// Is this point inside of the shape of this object ? (which can be complex)

	virtual bool mousePressEvent(QMouseEvent*) { return false; }	// Return true to capture the mouse for this object
	virtual void mouseMoveEvent(QMouseEvent*) {}
	virtual void mouseReleaseEvent(QMouseEvent*) {}

	virtual bool acceptsMagneticSnap() const; // If this object is used for the magnetic snap when moving objects

	virtual panda::types::Point getObjectSize();
	
	const std::vector<DataRectPair>& getDataRects() const;
	panda::BaseData* getDataAtPos(const panda::types::Point &pt, panda::types::Point* center = nullptr) const;
	bool getDataRect(const panda::BaseData* data, panda::types::Rect& rect) const;

	panda::types::Rect getVisualArea() const;
	panda::types::Rect getSelectionArea() const;
	panda::types::Point getPosition() const;
	panda::PandaObject* const getObject() const;
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

	virtual panda::types::Rect getTextArea(); // The area in which we can render text
	virtual std::string getLabel() const; // The text to draw

	void drawData(graphics::DrawList& list, graphics::DrawColors& colors, const panda::BaseData* data, const panda::types::Rect& area);
	
	panda::PandaObject* m_object;
	panda::types::Rect m_visualArea, m_selectionArea;
	std::vector<DataRectPair> m_datas;

	std::string m_currentLabel;
	graphics::DrawList m_textDrawList;
	graphics::DrawPath m_outline;
	graphics::DrawMesh m_fillShape;

	std::unique_ptr<panda::msg::Observer> m_observer;

private:
	void positionChanged(panda::types::Point newPos);

	GraphView* m_parentView;
	ObjectPositionAddon& m_positionAddon;
	panda::types::Point m_position;
	bool m_dirty = true;
};

inline bool ObjectRenderer::acceptsMagneticSnap() const
{ return true; }

inline bool ObjectRenderer::contains(const panda::types::Point& point)
{ return m_selectionArea.contains(point) && m_outline.contains(point); }

inline panda::types::Rect ObjectRenderer::getVisualArea() const
{ return m_visualArea; }

inline panda::types::Rect ObjectRenderer::getSelectionArea() const
{ return m_selectionArea; }

inline panda::types::Point ObjectRenderer::getPosition() const
{ return m_position; }

inline panda::PandaObject* const ObjectRenderer::getObject() const
{ return m_object; }

inline GraphView* const ObjectRenderer::getParentView() const
{ return m_parentView; }

inline int ObjectRenderer::dataStartY()
{ return dataRectMargin; }

inline const std::vector<ObjectRenderer::DataRectPair>& ObjectRenderer::getDataRects() const
{ return m_datas; }

//****************************************************************************//

class BaseObjectDrawCreator
{
public:
	virtual ~BaseObjectDrawCreator() {}
	virtual const panda::BaseClass* getClass() const = 0;
	virtual std::shared_ptr<ObjectRenderer> create(GraphView* view, panda::PandaObject* obj) const = 0;
};

template<class O, class D>
class ObjectDrawCreator : public BaseObjectDrawCreator
{
public:
	ObjectDrawCreator()
		: theClass(O::GetClass())
	{ }

	const panda::BaseClass* getClass() const override
	{ return theClass; }

	std::shared_ptr<ObjectRenderer> create(GraphView* view, panda::PandaObject* obj) const override
	{ return std::make_shared<D>(view, dynamic_cast<O*>(obj)); }

protected:
	const panda::BaseClass* theClass;
};

class ObjectRendererFactory
{
public:
	static ObjectRendererFactory* getInstance();
	std::shared_ptr<ObjectRenderer> createRenderer(GraphView *view, panda::PandaObject *obj);

protected:
	void addCreator(BaseObjectDrawCreator* creator);

	std::vector< std::shared_ptr<BaseObjectDrawCreator> > creators;

	template <class O, class D> friend class RegisterDrawObject;
};

template <class O, class D>
class RegisterDrawObject
{
public:
	RegisterDrawObject() {}
	operator int()
	{
		ObjectRendererFactory::getInstance()->addCreator(new ObjectDrawCreator<O, D>());
		return 1;
	}
};

} // namespace object

} // namespace graphview
