#ifndef GROUP_H
#define GROUP_H

#include <panda/PandaObject.h>
#include <ui/ObjectDrawStruct.h>

class GraphView;

namespace panda
{

class PandaDocument;

class Group : public PandaObject
{
    Q_OBJECT
public:
    explicit Group(PandaDocument *parent = 0);
	virtual ~Group();

    static bool createGroup(PandaDocument* doc, GraphView* view);
	static bool ungroupSelection(PandaDocument* doc, GraphView* view);

    virtual void save(QDataStream& out);
    virtual void save(QTextStream& out);

    virtual void load(QDataStream& in);
    virtual void load(QTextStream& in);

protected:
    Data<QString> groupName;

    QList<PandaObject*> objects;
    QMap<PandaObject*, QPointF> positions;
    QList< QSharedPointer<BaseData> > groupDatas;

    BaseData* duplicateData(BaseData* data);
    void addInputData(BaseData* from, BaseData* to);
    void addOutputData(BaseData* from, BaseData* to);
};

} // namespace panda

class GroupObjectDrawStruct : public ObjectDrawStruct
{
public:
	GroupObjectDrawStruct(GraphView* view, panda::PandaObject* object);

	virtual void drawShape(QPainter* painter);
	virtual void moveVisual(const QPointF& delta);
	virtual void update();
	virtual bool contains(const QPointF& point);
	virtual int dataStartY();

protected:
	QPainterPath shapePath;
};

#endif // GROUP_H
