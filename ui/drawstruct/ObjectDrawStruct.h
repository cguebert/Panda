#ifndef OBJECTDRAWSTRUCT_H
#define OBJECTDRAWSTRUCT_H

#include <QWidget>

#include <memory>

class GraphView;

namespace panda
{
	class PandaObject;
	class BaseData;
	class BaseClass;
	class XmlElement;
}

class ObjectDrawStruct
{
public:
	using RectDataPair = std::pair<QRectF, panda::BaseData*>;

	ObjectDrawStruct(GraphView* view, panda::PandaObject* obj);

	virtual void drawBackground(QPainter*) {}	// First called
	virtual void draw(QPainter* painter, bool selected = false);		// "Normal" draw
	virtual void drawForeground(QPainter*) {}	// Last called

	// The next 3 functions are here if we want to replace only part of the normal draw
	virtual void drawShape(QPainter* painter);
	virtual void drawDatas(QPainter* painter);
	virtual void drawText(QPainter* painter);

	virtual void update();							// Recompute the information about this object
	virtual void move(const QPointF& delta);		// Move the position of the object in the view
	virtual void moveVisual(const QPointF& delta);	// The view is moving (not the position of the object)
	virtual bool contains(const QPointF& point);	// Is this point inside of the shape of this object ? (which can be complex)

	virtual void save(panda::XmlElement& elem);
	virtual void load(const panda::XmlElement& elem);

	virtual bool mousePressEvent(QMouseEvent*) { return false; }	// Return true to capture the mouse for this object
	virtual void mouseMoveEvent(QMouseEvent*) {}
	virtual void mouseReleaseEvent(QMouseEvent*) {}

	virtual bool acceptsMagneticSnap() const; // If this object is used for the magnetic snap when moving objects

	virtual QSize getObjectSize();
	virtual QRectF getTextArea(); // The area in which we can render text

	const std::vector<RectDataPair>& getDataRects() const;
	panda::BaseData* getDataAtPos(const QPointF &pt, QPointF* center = nullptr) const;
	bool getDataRect(const panda::BaseData* data, QRectF& rect) const;

	QRectF getObjectArea() const;
	QPointF getPosition() const;
	panda::PandaObject* const getObject() const;

	static const int objectDefaultWidth = 100;
	static const int objectDefaultHeight = 50;

	static const int dataRectSize = 10;
	static const int dataRectMargin = 5;

	static const int objectCorner = 5;

	virtual int dataStartY();

protected:
	void drawData(QPainter* painter, const panda::BaseData* data, const QRectF& area);

	GraphView* m_parentView;
	panda::PandaObject* m_object;
	QPointF m_position;
	QRectF m_objectArea;

	std::vector<RectDataPair> m_datas;
};

inline bool ObjectDrawStruct::acceptsMagneticSnap() const
{ return true; }

inline bool ObjectDrawStruct::contains(const QPointF& point)
{ return m_objectArea.contains(point); }

inline QRectF ObjectDrawStruct::getObjectArea() const
{ return m_objectArea; }

inline QPointF ObjectDrawStruct::getPosition() const
{ return m_position; }

inline panda::PandaObject* const ObjectDrawStruct::getObject() const
{ return m_object; }

inline int ObjectDrawStruct::dataStartY()
{ return dataRectMargin; }

inline const std::vector<ObjectDrawStruct::RectDataPair>& ObjectDrawStruct::getDataRects() const
{ return m_datas; }

//****************************************************************************//

class BaseObjectDrawCreator
{
public:
	virtual ~BaseObjectDrawCreator() {}
	virtual const panda::BaseClass* getClass() const = 0;
	virtual std::shared_ptr<ObjectDrawStruct> create(GraphView* view, panda::PandaObject* obj) const = 0;
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

	std::shared_ptr<ObjectDrawStruct> create(GraphView* view, panda::PandaObject* obj) const override
	{ return std::make_shared<D>(view, dynamic_cast<O*>(obj)); }

protected:
	const panda::BaseClass* theClass;
};

class ObjectDrawStructFactory
{
public:
	static ObjectDrawStructFactory* getInstance();
	std::shared_ptr<ObjectDrawStruct> createDrawStruct(GraphView *view, panda::PandaObject *obj);

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
		ObjectDrawStructFactory::getInstance()->addCreator(new ObjectDrawCreator<O, D>());
		return 1;
	}
};

#endif
