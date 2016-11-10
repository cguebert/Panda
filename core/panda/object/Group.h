#ifndef GROUP_H
#define GROUP_H

#include <panda/object/PandaObject.h>
#include <panda/object/Layer.h>
#include <panda/types/Point.h>
#include <panda/document/DocumentDatas.h>
#include <panda/document/ObjectsList.h>
#include <panda/messaging.h>

namespace panda
{

class EditGroupCommand;
class PandaDocument;
class RenderedDocument;

class PANDA_CORE_API Group : public PandaObject
{
public:
	PANDA_CLASS(Group, PandaObject)

	explicit Group(PandaDocument* parent = nullptr);
	virtual ~Group();

	void save(XmlElement& elem, const std::vector<PandaObject*>* selected = nullptr) override;
	void load(const XmlElement& elem) override;

	void reset() override;

	const std::string& getGroupName() const;
	Data<std::string>& getGroupNameData();
	std::string getLabel() const override;

	ObjectsList& getObjectsList();

	void beginStep() override;
	void endStep() override;
	void preDestruction() override;

	using DataPtr = std::shared_ptr<BaseData>;

	DocumentDatas& groupDatas();

	std::string findAvailableDataName(const std::string& baseName, BaseData* data = nullptr);
	DataPtr duplicateData(BaseData* data);

protected:
	Data<std::string> m_groupName;

	ObjectsList m_objectsList;
	DocumentDatas m_groupDatas;
};

inline const std::string& Group::getGroupName() const
{ return m_groupName.getValue(); }

inline Data<std::string>& Group::getGroupNameData()
{ return m_groupName; }

inline std::string Group::getLabel() const
{ return m_groupName.getValue(); }

inline ObjectsList& Group::getObjectsList()
{ return m_objectsList; }

inline DocumentDatas& Group::groupDatas()
{ return m_groupDatas; }

//****************************************************************************//

class PANDA_CORE_API GroupWithLayer : public Group, public BaseLayer
{
public:
	PANDA_CLASS(GroupWithLayer, Group)

	explicit GroupWithLayer(RenderedDocument* parent);
	void setLayer(Layer* m_layer);

	void update() override;

	RenderersList getRenderers() override;

	const std::string getLayerName() const override;
	Data<std::string>& getLayerNameData() override;

	int getCompositionMode() const override;
	Data<int>& getCompositionModeData() override;

	float getOpacity() const override;
	Data<float>& getOpacityData() override;

	Data<types::ImageWrapper>* getImage() override;

	graphics::Mat4x4& getMVPMatrix() override;
	graphics::Size getLayerSize() const override;

	void addedObject(PandaObject* object);
	void removedObject(PandaObject* object);

	void removedFromDocument() override;

protected:
	Data<types::ImageWrapper> m_image;
	Data<int> m_compositionMode;
	Data<float> m_opacity;

	graphics::Mat4x4 m_mvpMatrix;

	Layer* m_layer;
	RenderersList m_renderers;

	msg::Observer m_observer;
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

	std::string getLabel() const override
	{ return m_caption.getValue(); }

	virtual bool hasConnectedInput() = 0;
	virtual bool hasConnectedOutput() = 0;

	virtual BaseData* getInputUserData() = 0;
	virtual BaseData* getOutputUserData() = 0;

protected:
	Data<std::string> m_caption;
};

//****************************************************************************//

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
