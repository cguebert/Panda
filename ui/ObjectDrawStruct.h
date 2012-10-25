#ifndef OBJECTDRAWSTRUCT_H
#define OBJECTDRAWSTRUCT_H

#include <QWidget>

class GraphView;

namespace panda
{
	class PandaObject;
	class BaseData;
}

class ObjectDrawStruct
{
public:
	ObjectDrawStruct(GraphView* view, panda::PandaObject* obj);

	virtual void draw(QPainter* painter);
	virtual void drawShape(QPainter* painter);
	virtual void drawDatas(QPainter* painter);
	virtual void drawText(QPainter* painter);
	virtual void drawLinks(QPainter* painter);
	virtual void update();							// Recompute the information about this object
	virtual void move(const QPointF& delta);		// Move the position of the object in the view
	virtual void moveVisual(const QPointF& delta);	// The view is moving (not the position of the object)
	virtual bool contains(const QPointF& point);	// Is this point inside of the shape of this object ? (which can be complex)

	virtual QSize getObjectSize();
	panda::BaseData* getDataAtPos(const QPointF &pt, QPointF* center = NULL);
	bool getDataRect(panda::BaseData* data, QRectF& rect);

	QRectF getObjectArea() const;
	QPointF getPosition() const;

	typedef QPair<QRectF, panda::BaseData*> RectDataPair;
	typedef QListIterator<RectDataPair> RectDataIterator;
	RectDataIterator getDatasIterator() const;

	static const int objectDefaultWidth = 100;
	static const int objectDefaultHeight = 50;

	static const int dataRectSize = 10;
	static const int dataRectMargin = 5;
protected:
	GraphView* parentView;
	panda::PandaObject* object;
	QPointF position;
	QRectF objectArea;
	QList<RectDataPair> datas;
};

class BaseObjectDrawCreator
{
public:
	virtual ~BaseObjectDrawCreator() {}
	virtual bool match(panda::PandaObject*) const = 0;
	virtual ObjectDrawStruct* create(GraphView* view, panda::PandaObject* obj) const = 0;
};

template<class O, class D>
class ObjectDrawCreator : public BaseObjectDrawCreator
{
public:
	virtual bool match(panda::PandaObject* obj) const
	{
		return dynamic_cast<O*>(obj) != NULL;
	}

	virtual ObjectDrawStruct* create(GraphView* view, panda::PandaObject* obj) const
	{
		return new D(view, dynamic_cast<O*>(obj));
	}
};

class ObjectDrawStructFactory
{
public:
	static ObjectDrawStructFactory* getInstance();
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
