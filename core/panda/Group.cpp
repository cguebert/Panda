#include <panda/DataFactory.h>
#include <panda/Group.h>
#include <panda/Layer.h>
#include <panda/ObjectFactory.h>
#include <panda/PandaDocument.h>
#include <panda/Renderer.h>
#include <panda/SimpleGUI.h>
#include <panda/command/GroupCommand.h>
#include <panda/command/LinkDatasCommand.h>
#include <panda/helper/algorithm.h>

namespace panda
{

Group::Group(PandaDocument* parent)
	: PandaObject(parent)
	, m_groupName(initData(std::string("Group"), "name", "Name to be displayed for this group"))
{
}

Group::~Group()
{
}

void Group::addObject(ObjectPtr object)
{
	if (!helper::contains(m_objects, object))
		m_objects.push_back(object);
}

void Group::save(XmlElement& elem, const std::vector<PandaObject*>* selected)
{
	// Saving group datas
	for(const auto& data : m_groupDatas)
	{
		auto node = elem.addChild("GroupData");
		node.setAttribute("type", DataFactory::typeToName(data->getDataTrait()->fullTypeId()));
		node.setAttribute("input", data->isInput());
		node.setAttribute("output", data->isOutput());
		node.setAttribute("name", data->getName());
		node.setAttribute("help", data->getHelp());

		const auto widget = data->getWidget();
		const auto widgetData = data->getWidgetData();
		if(!widget.empty())
			node.setAttribute("widget", widget);
		if(!widgetData.empty())
			node.setAttribute("widgetData", widgetData);
	}

	// Saving data values
	PandaObject::save(elem, selected);

	typedef QPair<BaseData*, BaseData*> DataPair;
	std::vector<DataPair> links;

	typedef QPair<quint32, quint32> IntPair;
	std::vector<IntPair> dockedObjects;

	PandaDocument::ObjectsSelection allObjects;
	for(auto object : m_objects)
		allObjects.push_back(object.get());
	allObjects.push_back(this);

	// Saving objects in this group
	for(auto object : m_objects)
	{
		auto node = elem.addChild("Object");
		node.setAttribute("type", ObjectFactory::getRegistryName(object.get()));
		node.setAttribute("index", object->getIndex());

		object->save(node, &allObjects);

		QPointF pos = m_positions[object.get()];
		node.setAttribute("x", pos.x());
		node.setAttribute("y", pos.y());

		// Preparing links
		for(BaseData* data : object->getInputDatas())
		{
			BaseData* parent = data->getParent();
			if(parent)
				links.push_back(qMakePair(data, parent));
		}

		// Preparing dockables list for docks
		DockObject* dock = dynamic_cast<DockObject*>(object.get());
		if(dock)
		{
			for(auto dockable : dock->getDockedObjects())
				dockedObjects.push_back(qMakePair(dock->getIndex(), dockable->getIndex()));
		}
	}

	// Links from objects to the output of the group
	for(BaseData* data : getOutputDatas())
	{
		BaseData* parent = data->getParent();
		if(parent)
			links.push_back(qMakePair(data, parent));
	}

	// Saving links
	for(DataPair link : links)
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
	for(IntPair dockable : dockedObjects)
	{
		auto node = elem.addChild("Dock");
		node.setAttribute("dock", dockable.first);
		node.setAttribute("docked", dockable.second);
	}
}

void Group::load(XmlElement& elem)
{
	auto groupDataNode = elem.firstChild("GroupData");
	while(groupDataNode)
	{
		quint32 type, input, output;
		std::string name, help, widget, widgetData;
		type = DataFactory::nameToType(groupDataNode.attribute("type").toString());
		input = groupDataNode.attribute("input").toUnsigned();
		output = groupDataNode.attribute("output").toUnsigned();
		name = groupDataNode.attribute("name").toString();
		help = groupDataNode.attribute("help").toString();
		widget = groupDataNode.attribute("widget").toString();
		widgetData = groupDataNode.attribute("widgetData").toString();

		auto dataPtr = DataFactory::getInstance()->create(type, name, help, this);
		auto data = dataPtr.get();
		if(!widget.empty())
			data->setWidget(widget);
		if(!widgetData.empty())
			data->setWidgetData(widgetData);
		m_groupDatas.push_back(dataPtr);
		if(input)
			addInput(*data);
		if(output)
			addOutput(*data);

		groupDataNode = groupDataNode.nextSibling("GroupData");
	}

	// Loading data values
	PandaObject::load(elem);

	std::map<quint32, PandaObject*> importObjectsMap;
	ObjectFactory* factory = ObjectFactory::getInstance();

	auto objectNode = elem.firstChild("Object");
	while(objectNode)
	{
		std::string registryName = objectNode.attribute("type").toString();
		quint32 index = objectNode.attribute("index").toUnsigned();
		auto object = factory->create(registryName, m_parentDocument);
		if(object)
		{
			importObjectsMap[index] = object.get();
			addObject(object);

			object->load(objectNode);

			QPointF pos;
#ifdef PANDA_DOUBLE
			pos.setX(objectNode.attribute("x").toDouble());
			pos.setY(objectNode.attribute("y").toDouble());
#else
			pos.setX(objectNode.attribute("x").toFloat());
			pos.setY(objectNode.attribute("y").toFloat());
#endif
			m_positions[object.get()] = pos;
		}
		else
		{
			getParentDocument()->getGUI().messageBox(gui::MessageBoxType::warning, "Panda", "Could not create the object " + registryName + ".\nA plugin must be missing.");
			return;
		}

		objectNode = objectNode.nextSibling("Object");
	}

	// Create links
	auto linkNode = elem.firstChild("Link");
	while(linkNode)
	{
		quint32 index1, index2;
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

		linkNode = linkNode.nextSibling("Link");
	}

	// Put dockables in their docks
	auto dockNode = elem.firstChild("Dock");
	while(dockNode)
	{
		quint32 dockIndex, dockableIndex;
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

		dockNode = dockNode.nextSibling("Dock");
	}

	m_parentDocument->onModifiedObject(this);
}

void Group::reset()
{
	for(auto object : m_objects)
		object->reset();
}

void Group::beginStep()
{
	PandaObject::beginStep();
	for(auto object : m_objects)
		object->beginStep();
}

void Group::endStep()
{
	PandaObject::endStep();
	for(auto object : m_objects)
		object->endStep();
}

int GroupClass = RegisterObject<Group>("Group").setDescription("Groups many object into a single one").setHidden(true);

//****************************************************************************//

GroupWithLayer::GroupWithLayer(PandaDocument* parent)
	: Group(parent)
	, m_layer(nullptr)
	, m_image(initData("image", "Image created by the renderers connected to this layer"))
	, m_compositionMode(initData(0, "composition mode", "Defines how this layer is merged on top of the previous ones (see help for list of modes)"))
	, m_opacity(initData((PReal)1.0, "opacity", "Set the opacity of the layer"))
{
	addInput(m_compositionMode);
	addInput(m_opacity);

	addOutput(*parent);

	m_compositionMode.setInput(false);
	m_compositionMode.setDisplayed(false);
	m_opacity.setInput(false);
	m_opacity.setDisplayed(false);
}

void GroupWithLayer::setLayer(Layer* newLayer)
{
	m_layer = newLayer;

	// Reinsert the group where the layer was
	if(m_layer)
	{
		int layerPos = m_parentDocument->getObjectPosition(m_layer);
		if(layerPos != -1)
			m_parentDocument->reinsertObject(this, layerPos);
	}
}

void GroupWithLayer::update()
{
	updateLayer(m_parentDocument);
	cleanDirty();
}

BaseLayer::RenderersList GroupWithLayer::getRenderers()
{
	if(m_layer)
		return m_layer->getRenderers();
	else
		return m_renderers;
}

void GroupWithLayer::addObject(ObjectPtr object)
{
	Group::addObject(object);

	Layer* layer = dynamic_cast<Layer*>(object.get());
	if(layer)
	{
		setLayer(layer);
		return;
	}

	Layer* defaultLayer = m_parentDocument->getDefaultLayer();
	Renderer* renderer = dynamic_cast<Renderer*>(object.get());
	if(renderer)
	{
		if(renderer->getParentDock() == defaultLayer)
			defaultLayer->removeDockable(renderer);
		renderer->setParentDock(nullptr);
		addInput(*renderer);

		// Sort the renderers by their position in the view
		auto rpos = m_positions[renderer].y();
		auto iter = std::find_if(m_renderers.begin(), m_renderers.end(), [&](Renderer* val){
			auto lpos = m_positions[val].y();
			return lpos > rpos;
		});
		m_renderers.insert(iter, renderer);
	}
}

void GroupWithLayer::removeObject(PandaObject* object)
{
	Group::removeObject(object);

	Renderer* renderer = dynamic_cast<Renderer*>(object);
	if(renderer && !renderer->getParentDock())
		m_parentDocument->getDefaultLayer()->addDockable(renderer);
}

void GroupWithLayer::removedFromDocument()
{
	PandaObject::removedFromDocument();

	// Reinsert the layer where the group was
	if(m_layer)
	{
		int layerPos = m_parentDocument->getObjectPosition(this);
		m_parentDocument->reinsertObject(m_layer, layerPos);
	}
}

graphics::Size GroupWithLayer::getLayerSize() const
{
	if(m_layer)
		return m_layer->getLayerSize();
	else
		return m_parentDocument->getRenderSize();
}

int GroupWithLayerClass = RegisterObject<GroupWithLayer>("GroupWithLayer").setDescription("Groups many object into a single one (version with a layer)").setHidden(true);

} // namespace panda
