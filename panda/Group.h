#ifndef GROUP_H
#define GROUP_H

#include <panda/PandaObject.h>
#include <panda/Layer.h>

class EditGroupDialog;
class GraphView;

namespace panda
{

class PandaDocument;

class Group : public QObject, public PandaObject
{
	Q_OBJECT
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

	typedef QSharedPointer<PandaObject> ObjectPtr;
	typedef QVector<ObjectPtr> ObjectsList;
	const ObjectsList& getObjects() const;

	virtual void addObject(ObjectPtr obj);
	virtual void removeObject(PandaObject*) {}

protected:
	Data<QString> m_groupName;

	ObjectsList m_objects;
	QMap<PandaObject*, QPointF> m_positions;
	QList< QSharedPointer<BaseData> > m_groupDatas;

	BaseData* duplicateData(BaseData* data);
	QString findAvailableDataName(QString baseName, BaseData* data=nullptr);
};

class GroupWithLayer : public Group, public BaseLayer
{
public:
	PANDA_CLASS(GroupWithLayer, Group)

	explicit GroupWithLayer(PandaDocument* parent);
	void setLayer(Layer* m_layer);

	virtual void update();

	virtual RenderersList getRenderers();

	virtual QString getLayerName() const;
	virtual Data<QString>& getLayerNameData();

	virtual int getCompositionMode() const;
	virtual Data<int>& getCompositionModeData();

	virtual PReal getOpacity() const;
	virtual Data<PReal>& getOpacityData();

	virtual Data<types::ImageWrapper>* getImage();
	virtual QMatrix4x4& getMVPMatrix();

	virtual void addObject(ObjectPtr obj);
	virtual void removeObject(PandaObject* obj);

protected:
	Data<types::ImageWrapper> m_image;
	Data<int> m_compositionMode;
	Data<PReal> m_opacity;

	QMatrix4x4 m_mvpMatrix;

	Layer* m_layer;
};

} // namespace panda

#endif // GROUP_H
