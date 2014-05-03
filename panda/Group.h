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
	PANDA_CLASS(Group, PandaObject)

	explicit Group(PandaDocument *parent = 0);
	virtual ~Group();

	static bool createGroup(PandaDocument* doc, GraphView* view);
	static bool ungroupSelection(PandaDocument* doc, GraphView* view);

	virtual void save(QDomDocument& doc, QDomElement& elem, const QList<PandaObject*>* selected = nullptr);
	virtual void load(QDomElement& elem);

	virtual void reset();

	QString getGroupName();

	typedef QList<PandaObject*> ObjectsList;
	const ObjectsList& getObjects() const;

protected:
	Data<QString> groupName;

	ObjectsList objects;
	QMap<PandaObject*, QPointF> positions;
	QList< QSharedPointer<BaseData> > groupDatas;

	virtual void addObject(PandaObject* obj);
	virtual void removeObject(PandaObject*) {}

	BaseData* duplicateData(BaseData* data);
	QString findAvailableDataName(QString baseName, BaseData* data=nullptr);
};

class GroupWithLayer : public Group, public BaseLayer
{
public:
	PANDA_CLASS(GroupWithLayer, Group)

	explicit GroupWithLayer(PandaDocument* parent);
	void setLayer(Layer* layer);

	virtual void update();

	virtual QList<Renderer*> getRenderers();

	virtual QString getLayerName() const;
	virtual void setLayerName(QString name);

	virtual int getCompositionMode() const;
	virtual void setCompositionMode(int mode);

	virtual PReal getOpacity() const;
	virtual void setOpacity(PReal opa);

	virtual Data<types::ImageWrapper>* getImage();
	virtual QMatrix4x4& getMVPMatrix();

protected:
	virtual void addObject(PandaObject* obj);
	virtual void removeObject(PandaObject* obj);

	Data<types::ImageWrapper> image;
	QMatrix4x4 mvpMatrix;
	int compositionMode;
	PReal opacity;

	Layer* layer;
};

} // namespace panda

#endif // GROUP_H
