#ifndef GROUP_H
#define GROUP_H

#include <panda/PandaObject.h>

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

#endif // GROUP_H
