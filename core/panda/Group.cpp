#include <panda/Group.h>
#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>
#include <panda/Layer.h>
#include <panda/Renderer.h>
#include <panda/DataFactory.h>
#include <panda/command/GroupCommand.h>
#include <panda/command/LinkDatasCommand.h>

#include <QMessageBox>

namespace panda
{

Group::Group(PandaDocument* parent)
	: PandaObject(parent)
	, m_groupName(initData(&m_groupName, QString("Group"), "name", "Name to be displayed for this group"))
{
}

Group::~Group()
{
}

void Group::save(QDomDocument& doc, QDomElement& elem, const QList<PandaObject*>* selected)
{
	// Saving group datas
	for(QSharedPointer<BaseData> data : m_groupDatas)
	{
		QDomElement node = doc.createElement("GroupData");
		elem.appendChild(node);
		node.setAttribute("type", DataFactory::typeToName(data->getDataTrait()->fullTypeId()));
		node.setAttribute("input", data->isInput());
		node.setAttribute("output", data->isOutput());
		node.setAttribute("name", data->getName());
		node.setAttribute("help", data->getHelp());
	}

	// Saving data values
	PandaObject::save(doc, elem, selected);

	typedef QPair<BaseData*, BaseData*> DataPair;
	QList<DataPair> links;

	typedef QPair<quint32, quint32> IntPair;
	QList<IntPair> dockedObjects;

	PandaDocument::ObjectsSelection allObjects;
	for(auto object : m_objects)
		allObjects.push_back(object.data());
	allObjects.push_back(this);

	// Saving objects in this group
	for(auto object : m_objects)
	{
		QDomElement node = doc.createElement("Object");
		node.setAttribute("type", ObjectFactory::getRegistryName(object.data()));
		node.setAttribute("index", object->getIndex());
		elem.appendChild(node);

		object->save(doc, node, &allObjects);

		QPointF pos = m_positions[object.data()];
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
		DockObject* dock = dynamic_cast<DockObject*>(object.data());
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
		QDomElement node = doc.createElement("Link");
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
		elem.appendChild(node);
	}

	// Saving docked objects list
	for(IntPair dockable : dockedObjects)
	{
		QDomElement node = doc.createElement("Dock");
		node.setAttribute("dock", dockable.first);
		node.setAttribute("docked", dockable.second);
		elem.appendChild(node);
	}
}

void Group::load(QDomElement& elem)
{
	QDomElement groupDataNode = elem.firstChildElement("GroupData");
	while(!groupDataNode.isNull())
	{
		quint32 type, input, output;
		QString name, help;
		type = DataFactory::nameToType(groupDataNode.attribute("type"));
		input = groupDataNode.attribute("input").toUInt();
		output = groupDataNode.attribute("output").toUInt();
		name = groupDataNode.attribute("name");
		help = groupDataNode.attribute("help");

		auto dataPtr = DataFactory::getInstance()->create(type, name, help, this);
		auto data = dataPtr.data();
		m_groupDatas.push_back(dataPtr);
		if(input)
			addInput(data);
		if(output)
			addOutput(data);

		groupDataNode = groupDataNode.nextSiblingElement("GroupData");
	}

	// Loading data values
	PandaObject::load(elem);

	QMap<quint32, PandaObject*> importObjectsMap;
	ObjectFactory* factory = ObjectFactory::getInstance();

	QDomElement objectNode = elem.firstChildElement("Object");
	while(!objectNode.isNull())
	{
		QString registryName = objectNode.attribute("type");
		quint32 index = objectNode.attribute("index").toUInt();
		auto object = factory->create(registryName, m_parentDocument);
		if(object)
		{
			importObjectsMap[index] = object.data();
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
			m_positions[object.data()] = pos;
		}
		else
		{
			QMessageBox::warning(nullptr, tr("Panda"),
				tr("Could not create the object %1.\nA plugin must be missing.")
				.arg(registryName));
			return;
		}

		objectNode = objectNode.nextSiblingElement("Object");
	}

	// Create links
	QDomElement linkNode = elem.firstChildElement("Link");
	while(!linkNode.isNull())
	{
		quint32 index1, index2;
		QString name1, name2;
		index1 = linkNode.attribute("object1").toUInt();
		index2 = linkNode.attribute("object2").toUInt();
		name1 = linkNode.attribute("data1");
		name2 = linkNode.attribute("data2");

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

		linkNode = linkNode.nextSiblingElement("Link");
	}

	// Put dockables in their docks
	QDomElement dockNode = elem.firstChildElement("Dock");
	while(!dockNode.isNull())
	{
		quint32 dockIndex, dockableIndex;
		dockIndex = dockNode.attribute("dock").toUInt();
		dockableIndex = dockNode.attribute("docked").toUInt();

		DockObject* dock = dynamic_cast<DockObject*>(importObjectsMap[dockIndex]);
		DockableObject* dockable = dynamic_cast<DockableObject*>(importObjectsMap[dockableIndex]);
		if(dock && dockable)
		{
			DockObject* defaultDock = dockable->getDefaultDock();
			if(defaultDock)
				defaultDock->removeDockable(dockable);
			dock->addDockable(dockable);
		}

		dockNode = dockNode.nextSiblingElement("Dock");
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
	, m_image(initData(&m_image, "image", "Image created by the renderers connected to this layer"))
	, m_compositionMode(initData(&m_compositionMode, 0, "composition mode", "Defines how this layer is merged on top of the previous ones (see help for list of modes)"))
	, m_opacity(initData(&m_opacity, (PReal)1.0, "opacity", "Set the opacity of the layer"))
{
	addInput(&m_compositionMode);
	addInput(&m_opacity);

	addOutput((DataNode*)parent);

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

	Layer* layer = dynamic_cast<Layer*>(object.data());
	if(layer)
	{
		setLayer(layer);
		return;
	}

	Layer* defaultLayer = m_parentDocument->getDefaultLayer();
	Renderer* renderer = dynamic_cast<Renderer*>(object.data());
	if(renderer)
	{
		if(renderer->getParentDock() == defaultLayer)
			defaultLayer->removeDockable(renderer);
		renderer->setParentDock(nullptr);
		addInput((DataNode*)renderer);

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

QSize GroupWithLayer::getLayerSize() const
{
	if(m_layer)
		return m_layer->getLayerSize();
	else
		return m_parentDocument->getRenderSize();
}

int GroupWithLayerClass = RegisterObject<GroupWithLayer>("GroupWithLayer").setDescription("Groups many object into a single one (version with a layer)").setHidden(true);

} // namespace panda