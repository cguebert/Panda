#ifndef GROUP_H
#define GROUP_H

#include <panda/object/PandaObject.h>
#include <panda/object/Layer.h>
#include <panda/types/Point.h>

class EditGroupCommand;
namespace panda
{

class PandaDocument;

class PANDA_CORE_API Group : public PandaObject
{
	friend class EditGroupCommand;
public:
	PANDA_CLASS(Group, PandaObject)

	explicit Group(PandaDocument* parent = nullptr);
	virtual ~Group();

	void save(XmlElement& elem, const std::vector<PandaObject*>* selected = nullptr) override;
	bool load(XmlElement& elem) override;

	void reset() override;

	const std::string& getGroupName();

	typedef std::shared_ptr<PandaObject> ObjectPtr;
	typedef std::vector<ObjectPtr> ObjectsList;
	const ObjectsList& getObjects() const;

	virtual void addObject(ObjectPtr object);
	virtual void removeObject(PandaObject*) {} // Bugfix: we never remove objects from groups, there are freed if the group is destroyed

	void beginStep() override;
	void endStep() override;
	void preDestruction() override;

	typedef std::shared_ptr<BaseData> DataPtr;
	typedef std::vector<DataPtr> GroupDataList;

	void addGroupData(DataPtr data);
	const GroupDataList& getGroupDatas(); // Can modify the datas, not the list

	void setPosition(PandaObject* object, types::Point pos);
	types::Point getPosition(PandaObject* object) const;

protected:
	Data<std::string> m_groupName;

	ObjectsList m_objects;
	std::map<PandaObject*, types::Point> m_positions;
	GroupDataList m_groupDatas;
};

inline const std::string& Group::getGroupName()
{ return m_groupName.getValue(); }

inline const Group::ObjectsList& Group::getObjects() const
{ return m_objects; }

inline void Group::addGroupData(DataPtr data)
{ m_groupDatas.push_back(data); }

inline const Group::GroupDataList& Group::getGroupDatas()
{ return m_groupDatas; }

inline void Group::setPosition(PandaObject* object, types::Point pos)
{ m_positions[object] = pos; }

inline types::Point Group::getPosition(PandaObject* object) const
{ return m_positions.at(object); }

//****************************************************************************//

class PANDA_CORE_API GroupWithLayer : public Group, public BaseLayer
{
public:
	PANDA_CLASS(GroupWithLayer, Group)

	explicit GroupWithLayer(PandaDocument* parent);
	void setLayer(Layer* m_layer);

	void update() override;

	RenderersList getRenderers() override;

	const std::string& getLayerName() const override;
	Data<std::string>& getLayerNameData() override;

	int getCompositionMode() const override;
	Data<int>& getCompositionModeData() override;

	float getOpacity() const override;
	Data<float>& getOpacityData() override;

	Data<types::ImageWrapper>* getImage() override;

	graphics::Mat4x4& getMVPMatrix() override;
	graphics::Size getLayerSize() const override;

	void addObject(ObjectPtr object) override;
	void removeObject(PandaObject* object) override;

	void removedFromDocument() override;

protected:
	Data<types::ImageWrapper> m_image;
	Data<int> m_compositionMode;
	Data<float> m_opacity;

	graphics::Mat4x4 m_mvpMatrix;

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
		, m_caption(initData("caption", "The caption to use in the graph view"))
	{ }

	const std::string& getCaption()
	{ return m_caption.getValue(); }

	virtual bool hasConnectedInput() = 0;
	virtual bool hasConnectedOutput() = 0;

	virtual BaseData* getInputUserData() = 0;
	virtual BaseData* getOutputUserData() = 0;

protected:
	Data<std::string> m_caption;
};

//****************************************************************************//

inline const std::string& GroupWithLayer::getLayerName() const
{ if(m_layer) return m_layer->getLayerName(); else return m_groupName.getValue(); }

inline Data<std::string>& GroupWithLayer::getLayerNameData()
{ if(m_layer) return m_layer->getLayerNameData(); else return m_groupName; }

inline int GroupWithLayer::getCompositionMode() const
{ if(m_layer) return m_layer->getCompositionMode(); else return m_compositionMode.getValue(); }

inline Data<int>& GroupWithLayer::getCompositionModeData()
{ if(m_layer) return m_layer->getCompositionModeData(); else return m_compositionMode; }

inline float GroupWithLayer::getOpacity() const
{ if(m_layer) return m_layer->getOpacity(); else return m_opacity.getValue(); }

inline Data<float>& GroupWithLayer::getOpacityData()
{ if(m_layer) return m_layer->getOpacityData(); else return m_opacity; }

inline Data<types::ImageWrapper>* GroupWithLayer::getImage()
{ if(m_layer) return m_layer->getImage(); else return &m_image; }

inline graphics::Mat4x4& GroupWithLayer::getMVPMatrix()
{ if(m_layer) return m_layer->getMVPMatrix(); else return m_mvpMatrix; }

} // namespace panda

#endif // GROUP_H
