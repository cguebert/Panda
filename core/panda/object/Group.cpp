#include <panda/document/RenderedDocument.h>
#include <panda/SimpleGUI.h>
#include <panda/data/DataFactory.h>
#include <panda/object/Group.h>
#include <panda/object/Layer.h>
#include <panda/object/ObjectAddons.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/Renderer.h>
#include <panda/command/GroupCommand.h>
#include <panda/command/LinkDatasCommand.h>
#include <panda/document/ObjectsList.h>
#include <panda/helper/algorithm.h>
#include <panda/helper/Exception.h>

namespace panda
{

Group::Group(PandaDocument* parent)
	: PandaObject(parent)
	, m_groupName(initData(std::string("Group"), "name", "Name to be displayed for this group"))
	, m_groupDatas(this)
{
}

Group::~Group()
{
}

void Group::save(XmlElement& elem, const std::vector<PandaObject*>* selected)
{
	// Saving group datas
	m_groupDatas.save(elem, "GroupData");

	// Saving data values
	PandaObject::save(elem, selected);

	typedef std::pair<BaseData*, BaseData*> DataPair;
	std::vector<DataPair> links;

	typedef std::pair<uint32_t, uint32_t> IntPair;
	std::vector<IntPair> dockedObjects;

	std::vector<PandaObject*> allObjects;
	for(auto object : m_objectsList.get())
		allObjects.push_back(object.get());
	allObjects.push_back(this);

	// Saving objects in this group
	for(auto object : m_objectsList.get())
	{
		auto node = elem.addChild("Object");
		node.setAttribute("type", ObjectFactory::getRegistryName(object.get()));
		node.setAttribute("index", object->getIndex());

		object->save(node, &allObjects);
		object->addons().save(node);

		// Preparing links
		for(BaseData* data : object->getInputDatas())
		{
			BaseData* parent = data->getParent();
			if(parent)
				links.push_back(std::make_pair(data, parent));
		}

		// Preparing dockables list for docks
		DockObject* dock = dynamic_cast<DockObject*>(object.get());
		if(dock)
		{
			for(auto dockable : dock->getDockedObjects())
				dockedObjects.push_back(std::make_pair(dock->getIndex(), dockable->getIndex()));
		}
	}

	// Links from objects to the output of the group
	for(BaseData* data : getOutputDatas())
	{
		BaseData* parent = data->getParent();
		if(parent)
			links.push_back(std::make_pair(data, parent));
	}

	// Saving links
	for(const DataPair& link : links)
	{
		auto node = elem.addChild("Link");
		if(link.first->getOwner() == this)
			node.setAttribute("object1", 0);
		else
			node.setAttribute("object1", link.first->getOwner()->getIndex());
		node.setAttribute("data1", link.first->getName());

		if(link.second->getOwner() == this)
			node.setAttribute("object2", 0);
		else
			node.setAttribute("object2", link.second->getOwner()->getIndex());
		node.setAttribute("data2", link.second->getName());
	}

	// Saving docked objects list
	for(const IntPair& dockable : dockedObjects)
	{
		auto node = elem.addChild("Dock");
		node.setAttribute("dock", dockable.first);
		node.setAttribute("docked", dockable.second);
	}
}

void Group::load(const XmlElement& elem)
{
	m_groupDatas.load(elem, "GroupData");

	// Loading data values
	PandaObject::load(elem);

	std::map<uint32_t, PandaObject*> importObjectsMap;
	ObjectFactory* factory = ObjectFactory::getInstance();

	for(auto objectNode = elem.firstChild("Object"); objectNode; objectNode = objectNode.nextSibling("Object"))
	{
		std::string registryName = objectNode.attribute("type").toString();
		uint32_t index = objectNode.attribute("index").toUnsigned();
		auto object = factory->create(registryName, parentDocument());
		if(object)
		{
			importObjectsMap[index] = object.get();
			m_objectsList.addObject(object);

			object->load(objectNode);
			object->addons().load(objectNode);
		}
		else
			throw panda::helper::Exception("Could not create the object " + registryName + ".\nA plugin must be missing.");
	}

	// Create links
	for(auto linkNode = elem.firstChild("Link"); linkNode; linkNode = linkNode.nextSibling("Link"))
	{
		uint32_t index1, index2;
		std::string name1, name2;
		index1 = linkNode.attribute("object1").toUnsigned();
		index2 = linkNode.attribute("object2").toUnsigned();
		name1 = linkNode.attribute("data1").toString();
		name2 = linkNode.attribute("data2").toString();

		PandaObject *object1, *object2;
		BaseData *data1=nullptr, *data2=nullptr;

		if(index1)
		{
			object1 = importObjectsMap[index1];
			if(object1)
				data1 = object1->getData(name1);
		}
		else
			data1 = getData(name1);

		if(index2)
		{
			object2 = importObjectsMap[index2];
			if(object2)
				data2 = object2->getData(name2);
		}
		else
			data2 = getData(name2);

		if(data1 && data2)
			data1->setParent(data2);
	}

	// Put dockables in their docks
	for(auto dockNode = elem.firstChild("Dock"); dockNode; dockNode = dockNode.nextSibling("Dock"))
	{
		uint32_t dockIndex, dockableIndex;
		dockIndex = dockNode.attribute("dock").toUnsigned();
		dockableIndex = dockNode.attribute("docked").toUnsigned();

		DockObject* dock = dynamic_cast<DockObject*>(importObjectsMap[dockIndex]);
		DockableObject* dockable = dynamic_cast<DockableObject*>(importObjectsMap[dockableIndex]);
		if(dock && dockable)
		{
			DockObject* defaultDock = dockable->getDefaultDock();
			if(defaultDock)
				defaultDock->removeDockable(dockable);
			dock->addDockable(dockable);
		}		
	}

	parentDocument()->onModifiedObject(this);
}

void Group::reset()
{
	for(auto& object : m_objectsList.get())
		object->reset();
}

void Group::beginStep()
{
	PandaObject::beginStep();
	for(auto& object : m_objectsList.get())
		object->beginStep();
}

void Group::endStep()
{
	PandaObject::endStep();
	for(auto& object : m_objectsList.get())
		object->endStep();
}

void Group::preDestruction()
{
	PandaObject::preDestruction();
	for (auto& object : m_objectsList.get())
		object->preDestruction();
}

std::string Group::findAvailableDataName(const std::string& baseName, BaseData* data)
{
	auto name = baseName;
	BaseData* testData = getData(name);
	if(testData && testData != data)
	{
		int i=2;
		testData = getData(name + std::to_string(i));
		while(testData && testData != data)
		{
			++i;
			testData = getData(name + std::to_string(i));
		}
		name = name + std::to_string(i);
	}
	return name;
}

Group::DataPtr Group::duplicateData(BaseData* data)
{
	auto name = findAvailableDataName(data->getName());

	auto newData = DataFactory::getInstance()->create(data->getDataTrait()->fullTypeId(),
										   name, data->getHelp(), this);
	newData->setDisplayed(data->isDisplayed());
	newData->setPersistent(data->isPersistent());
	newData->setWidget(data->getWidget());
	newData->setWidgetData(data->getWidgetData());
	newData->setInput(data->isInput());
	newData->setOutput(data->isOutput());

	return newData;
}

int GroupClass = RegisterObject<Group>("Group").setDescription("Groups many object into a single one").setHidden(true);

//****************************************************************************//

GroupWithLayer::GroupWithLayer(RenderedDocument* parent)
	: Group(parent)
	, m_layer(nullptr)
	, m_image(initData("image", "Image created by the renderers connected to this layer"))
	, m_compositionMode(initData(0, "composition mode", "Defines how this layer is merged on top of the previous ones (see help for list of modes)"))
	, m_opacity(initData((float)1.0, "opacity", "Set the opacity of the layer"))
{
	addInput(m_compositionMode);
	addInput(m_opacity);

	addOutput(*parent);

	m_compositionMode.setInput(false);
	m_compositionMode.setDisplayed(false);
	m_opacity.setInput(false);
	m_opacity.setDisplayed(false);

	m_observer.get(m_objectsList.addedObject).connect<GroupWithLayer, &GroupWithLayer::addedObject>(this);
	m_observer.get(m_objectsList.removedObject).connect<GroupWithLayer, &GroupWithLayer::removedObject>(this);
}

void GroupWithLayer::setLayer(Layer* newLayer)
{
	m_layer = newLayer;

	// Reinsert the group where the layer was
	if(m_layer)
	{
		int layerPos = parentDocument()->getObjectsList().getObjectPosition(m_layer);
		if(layerPos != -1)
			parentDocument()->getObjectsList().reinsertObject(this, layerPos);
	}
}

void GroupWithLayer::update()
{
	updateLayer(parent<RenderedDocument>());
}

BaseLayer::RenderersList GroupWithLayer::getRenderers()
{
	if(m_layer)
		return m_layer->getRenderers();
	else
		return m_renderers;
}

void GroupWithLayer::addedObject(PandaObject* object)
{
	Layer* layer = dynamic_cast<Layer*>(object);
	if(layer)
	{
		setLayer(layer);
		return;
	}

	Layer* defaultLayer = parent<RenderedDocument>()->getDefaultLayer();
	Renderer* renderer = dynamic_cast<Renderer*>(object);
	if(renderer)
	{
		if(renderer->getParentDock() == defaultLayer)
			defaultLayer->removeDockable(renderer);
		renderer->setParentDock(nullptr);
		addInput(*renderer);

		// Sort the renderers by their position in the view
	/*	auto rpos = m_positions[renderer].y;
		auto iter = std::find_if(m_renderers.begin(), m_renderers.end(), [&](Renderer* val){
			auto lpos = m_positions[val].y;
			return lpos > rpos;
		});
		m_renderers.insert(iter, renderer);*/
		m_renderers.push_back(renderer);
	}
}

void GroupWithLayer::removedObject(PandaObject* object)
{
	Renderer* renderer = dynamic_cast<Renderer*>(object);
	if(renderer && !renderer->getParentDock())
		parent<RenderedDocument>()->getDefaultLayer()->addDockable(renderer);
}

void GroupWithLayer::removedFromDocument()
{
	PandaObject::removedFromDocument();

	// Reinsert the layer where the group was
	if(m_layer)
	{
		int layerPos = parentDocument()->getObjectsList().getObjectPosition(this);
		parentDocument()->getObjectsList().reinsertObject(m_layer, layerPos);
	}
}

graphics::Size GroupWithLayer::getLayerSize() const
{
	if(m_layer)
		return m_layer->getLayerSize();
	else
		return parent<RenderedDocument>()->getRenderSize();
}

const std::string GroupWithLayer::getLayerName() const
{
	const auto& groupName = m_groupName.getValue();
	if(m_layer) 
		return groupName + '.' + m_layer->getLayerName(); 
	else 
		return groupName; 
}

int GroupWithLayerClass = RegisterObject<GroupWithLayer, RenderedDocument>("Group with Layer").setDescription("Groups many object into a single one (version with a layer)").setHidden(true);

} // namespace panda
