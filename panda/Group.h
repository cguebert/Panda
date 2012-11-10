#ifndef GROUP_H
#define GROUP_H

#include <panda/PandaObject.h>
#include <panda/Layer.h>

class GroupObjectDrawStruct;
class EditGroupDialog;
class GraphView;

namespace panda
{

class PandaDocument;

class Group : public PandaObject
{
    Q_OBJECT
	friend class GroupObjectDrawStruct;
	friend class EditGroupDialog;
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
	QString findAvailableDataName(QString baseName, BaseData* data=NULL);
};

class GroupWithLayer : public Group, public BaseLayer
{
public:
	explicit GroupWithLayer(PandaDocument* parent);
	void setLayer(Layer* layer);

	virtual void update();

	virtual QList<Renderer*> getRenderers();

	virtual QString getLayerName() const;
	virtual void setLayerName(QString name);

	virtual int getCompositionMode() const;
	virtual void setCompositionMode(int mode);

	virtual double getOpacity() const;
	virtual void setOpacity(double opa);

	virtual Data<QImage>* getImage();

protected:
	Data<QImage> image;
	Data<int> compositionMode;
	Data<double> opacity;

	Layer* layer;
};

} // namespace panda

#endif // GROUP_H
