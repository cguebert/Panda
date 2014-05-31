#ifndef OBJECTDRAWSTRUCT_H
#define OBJECTDRAWSTRUCT_H

#include <QWidget>
#include <QDomDocument>

class GraphView;

namespace panda
{
	class PandaObject;
	class BaseData;
	class BaseClass;
}

class ObjectDrawStruct
{
public:
	ObjectDrawStruct(GraphView* view, panda::PandaObject* obj);

	virtual void drawBackground(QPainter*) {}	// First called
	virtual void draw(QPainter* painter);		// "Normal" draw
	virtual void drawForeground(QPainter*) {}	// Last called

	// The next 3 functions are here if we want to replace only part of the normal draw
	virtual void drawShape(QPainter* painter);
	virtual void drawDatas(QPainter* painter);
	virtual void drawText(QPainter* painter);

	virtual void drawLinks(QPainter* painter);		// It is the job of the DrawStruct to draw the links coming to this object

	virtual void update();							// Recompute the information about this object
	virtual void move(const QPointF& delta);		// Move the position of the object in the view
	virtual void moveVisual(const QPointF& delta);	// The view is moving (not the position of the object)
	virtual bool contains(const QPointF& point);	// Is this point inside of the shape of this object ? (which can be complex)

	virtual void save(QDomDocument& doc, QDomElement& elem);
	virtual void load(QDomElement& elem);

	virtual bool mousePressEvent(QMouseEvent*) { return false; }	// Return true to capture the mouse for this object
	virtual void mouseMoveEvent(QMouseEvent*) {}
	virtual void mouseReleaseEvent(QMouseEvent*) {}

	virtual bool acceptsMagneticSnap(); // If this object is used for the magnetic snap when moving objects

	virtual QSize getObjectSize();
	panda::BaseData* getDataAtPos(const QPointF &pt, QPointF* center = nullptr);
	bool getDataRect(const panda::BaseData *data, QRectF& rect);

	QRectF getObjectArea() const;
	QPointF getPosition() const;
	panda::PandaObject* const getObject() const;

	static const int objectDefaultWidth = 100;
	static const int objectDefaultHeight = 50;

	static const int dataRectSize = 10;
	static const int dataRectMargin = 5;

	virtual int dataStartY();

protected:
	GraphView* m_parentView;
	panda::PandaObject* m_object;
	QPointF m_position;
	QRectF m_objectArea;

	typedef QPair<QRectF, panda::BaseData*> RectDataPair;
	QList<RectDataPair> m_datas;
};

inline bool ObjectDrawStruct::acceptsMagneticSnap()
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

//****************************************************************************//

class BaseObjectDrawCreator
{
public:
	virtual ~BaseObjectDrawCreator() {}
	virtual const panda::BaseClass* getClass() const = 0;
	virtual ObjectDrawStruct* create(GraphView* view, panda::PandaObject* obj) const = 0;
};

template<class O, class D>
class ObjectDrawCreator : public BaseObjectDrawCreator
{
public:
	ObjectDrawCreator()
		: theClass(O::getClass())
	{ }

	virtual const panda::BaseClass* getClass() const
	{
		return theClass;
	}

	virtual ObjectDrawStruct* create(GraphView* view, panda::PandaObject* obj) const
	{
		return new D(view, dynamic_cast<O*>(obj));
	}

protected:
	const panda::BaseClass* theClass;
};

class ObjectDrawStructFactory
{
public:
	static ObjectDrawStructFactory* getInstance();
	void debug();
	ObjectDrawStruct* createDrawStruct(GraphView *view, panda::PandaObject *obj);

protected:
	void addCreator(BaseObjectDrawCreator* creator);

	QList< QSharedPointer<BaseObjectDrawCreator> > creators;

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
