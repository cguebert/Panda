#ifndef GROUP_H
#define GROUP_H

#include <panda/PandaObject.h>
#include <panda/Layer.h>

class EditGroupCommand;
namespace panda
{

class PandaDocument;

class PANDA_CORE_API Group : public QObject, public PandaObject
{
	Q_OBJECT
	friend class EditGroupCommand;
public:
	PANDA_CLASS(Group, PandaObject)

	explicit Group(PandaDocument* parent = nullptr);
	virtual ~Group();

	virtual void save(QDomDocument& doc, QDomElement& elem, const QList<PandaObject*>* selected = nullptr);
	virtual void load(QDomElement& elem);

	virtual void reset();

	QString getGroupName();

	typedef QSharedPointer<PandaObject> ObjectPtr;
	typedef QVector<ObjectPtr> ObjectsList;
	const ObjectsList& getObjects() const;

	virtual void addObject(ObjectPtr object);
	virtual void removeObject(PandaObject*) {} // Bugfix: we never remove objects from groups, there are freed if the group is destroyed

	virtual void beginStep();
	virtual void endStep();

	typedef QSharedPointer<BaseData> DataPtr;
	typedef QList<DataPtr> GroupDataList;

	void addGroupData(DataPtr data);
	const GroupDataList& getGroupDatas(); // Can modify the datas, not the list

	void setPosition(PandaObject* object, QPointF pos);
	QPointF getPosition(PandaObject* object) const;

protected:
	Data<QString> m_groupName;

	ObjectsList m_objects;
	QMap<PandaObject*, QPointF> m_positions;
	GroupDataList m_groupDatas;
};

inline void Group::addGroupData(DataPtr data)
{ m_groupDatas.push_back(data); }

inline const Group::GroupDataList& Group::getGroupDatas()
{ return m_groupDatas; }

inline void Group::setPosition(PandaObject* object, QPointF pos)
{ m_positions[object] = pos; }

inline QPointF Group::getPosition(PandaObject* object) const
{ return m_positions.value(object); }

//****************************************************************************//

class PANDA_CORE_API GroupWithLayer : public Group, public BaseLayer
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
	virtual QSize getLayerSize() const;

	virtual void addObject(ObjectPtr object);
	virtual void removeObject(PandaObject* object);

	virtual void removedFromDocument();

protected:
	Data<types::ImageWrapper> m_image;
	Data<int> m_compositionMode;
	Data<PReal> m_opacity;

	QMatrix4x4 m_mvpMatrix;

	Layer* m_layer;
	RenderersList m_renderers;
};

//****************************************************************************//

class BaseGeneratorUser : public PandaObject
{
public:
	PANDA_CLASS(BaseGeneratorUser, PandaObject)

	BaseGeneratorUser(PandaDocument *doc)
		: PandaObject(doc)
		, m_caption(initData(&m_caption, "caption", "The caption to use in the graph view"))
	{ }

	const QString& getCaption()
	{ return m_caption.getValue(); }

	virtual bool hasConnectedInput() = 0;
	virtual bool hasConnectedOutput() = 0;

	virtual BaseData* getInputUserData() = 0;
	virtual BaseData* getOutputUserData() = 0;

protected:
	Data<QString> m_caption;
};

//****************************************************************************//

inline QString Group::getGroupName()
{ return m_groupName.getValue(); }

inline const Group::ObjectsList& Group::getObjects() const
{ return m_objects; }

inline void Group::addObject(ObjectPtr object)
{ if(!m_objects.contains(object)) m_objects.push_back(object); }

inline QString GroupWithLayer::getLayerName() const
{ if(m_layer) return m_layer->getLayerName(); else return m_groupName.getValue(); }

inline Data<QString>& GroupWithLayer::getLayerNameData()
{ if(m_layer) return m_layer->getLayerNameData(); else return m_groupName; }

inline int GroupWithLayer::getCompositionMode() const
{ if(m_layer) return m_layer->getCompositionMode(); else return m_compositionMode.getValue(); }

inline Data<int>& GroupWithLayer::getCompositionModeData()
{ if(m_layer) return m_layer->getCompositionModeData(); else return m_compositionMode; }

inline PReal GroupWithLayer::getOpacity() const
{ if(m_layer) return m_layer->getOpacity(); else return m_opacity.getValue(); }

inline Data<PReal>& GroupWithLayer::getOpacityData()
{ if(m_layer) return m_layer->getOpacityData(); else return m_opacity; }

inline Data<types::ImageWrapper>* GroupWithLayer::getImage()
{ if(m_layer) return m_layer->getImage(); else return &m_image; }

inline QMatrix4x4& GroupWithLayer::getMVPMatrix()
{ if(m_layer) return m_layer->getMVPMatrix(); else return m_mvpMatrix; }

} // namespace panda

#endif // GROUP_H
