#include "Group.h"

#include <ui/GraphView.h>
#include <ui/drawstruct/ObjectDrawStruct.h>

#include <ui/command/AddObjectCommand.h>
#include <ui/command/GroupCommand.h>
#include <ui/command/RemoveObjectCommand.h>
#include <ui/command/LinkDatasCommand.h>
#include <ui/command/MoveObjectCommand.h>

#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>
#include <panda/Layer.h>
#include <panda/Renderer.h>
#include <panda/DataFactory.h>

#include <QMessageBox>

namespace panda
{

Group::Group(PandaDocument *parent)
	: PandaObject(parent)
	, m_groupName(initData(&m_groupName, QString("Group"), "name", "Name to be displayed for this group"))
{
}

Group::~Group()
{
}

bool Group::createGroup(PandaDocument* doc, GraphView* view)
{
	if(doc->getNbSelected() < 2)
		return false;

	auto selection = doc->getSelection();
	bool hasRenderer = false;
	// Verify that all selected renderers are in the same layer
	Layer* layer = nullptr;
	for(auto object : selection)
	{
		Renderer* renderer = dynamic_cast<Renderer*>(object);
		if(renderer)
		{
			hasRenderer = true;
			if(!layer)
				layer = dynamic_cast<Layer*>(renderer->getParentDock());
			else if(layer != renderer->getParentDock())
			{
				QMessageBox::warning(nullptr, tr("Panda"),
					tr("All renderers must be placed in the same layer."));
				return false;
			}

			if(layer && layer!=doc->getDefaultLayer() && !doc->isSelected(layer))
			{
				QMessageBox::warning(nullptr, tr("Panda"),
					tr("Renderers must be grouped with their layers"));
				return false;
			}
		}
	}

	auto macro = doc->beginCommandMacro(tr("Create Group"));

	if(layer == doc->getDefaultLayer())	// Won't be added in the group!
		layer = nullptr;

	auto factory = ObjectFactory::getInstance();
	Group* group = nullptr;
	if(hasRenderer)
	{
		auto object = factory->create(ObjectFactory::getRegistryName<GroupWithLayer>(), doc);
		doc->addCommand(new AddObjectCommand(doc, view, object));
		auto groupWithLayer = dynamic_cast<GroupWithLayer*>(object.data());
		if(groupWithLayer)
			groupWithLayer->setLayer(layer);
		group = groupWithLayer;
	}
	else
	{
		auto object = factory->create(ObjectFactory::getRegistryName<Group>(), doc);
		doc->addCommand(new AddObjectCommand(doc, view, object));
		group = dynamic_cast<Group*>(object.data());
	}
	if(!group)
		return false;

	// Find center of the selection
	QRectF totalView;
	for(auto object : selection)
	{
		QRectF objectArea = view->getObjectDrawStruct(object)->getObjectArea();
		totalView = totalView.united(objectArea);
	}

	// Put the new object there
	ObjectDrawStruct* ods = view->getObjectDrawStruct(group);
	QSize objSize = ods->getObjectSize() / 2;
	ods->move(totalView.center() - view->getViewDelta() - ods->getPosition() - QPointF(objSize.width(), objSize.height()));
	QPointF groupPos = ods->getPosition();

	// If multiple outside datas are connected to the same data, merge them
	QMap<BaseData*, BaseData*> connectedInputDatas;

	// Adding the objects
	for(auto object : selection)
	{
		auto objectPtr = doc->getSharedPointer(object);
		if(!objectPtr)
			continue;
		doc->addCommand(new AddObjectToGroupCommand(group, objectPtr));

		// Storing the position of this object in respect to the group object
		QPointF delta = view->getObjectDrawStruct(object)->getPosition() - groupPos;
		group->m_positions[object] = delta;

		// Adding input datas
		for(BaseData* data : object->getInputDatas())
		{
			BaseData* otherData = data->getParent();
			if(otherData)
			{
				PandaObject* connected = otherData->getOwner();
				if(connected && !doc->isSelected(connected) && connected!=doc)
				{
					BaseData* createdData = nullptr;
					if(!connectedInputDatas.contains(otherData))
					{
						createdData = group->duplicateData(data);
						createdData->copyValueFrom(otherData);
						group->addInput(createdData);
						doc->addCommand(new LinkDatasCommand(createdData, otherData));
						connectedInputDatas.insert(otherData, createdData);
					}
					else
					{
						createdData = connectedInputDatas.value(otherData);
						QString name = group->findAvailableDataName(otherData->getName(), createdData);
						if(name != createdData->getName())
						{
							createdData->setName(name);
							createdData->setHelp(otherData->getHelp());
						}
					}

					if(createdData)
						doc->addCommand(new LinkDatasCommand(data, createdData));
				}
			}
		}

		// Adding ouput datas
		for(BaseData* data : object->getOutputDatas())
		{
			auto outputs = data->getOutputs();
			BaseData* createdData = nullptr;
			for(DataNode* otherNode : outputs)
			{
				BaseData* otherData = dynamic_cast<BaseData*>(otherNode);
				if(otherData)
				{
					PandaObject* connected = otherData->getOwner();
					if(connected && !doc->isSelected(connected) && connected!=doc)
					{
						if(!createdData)
						{
							createdData = group->duplicateData(data);
							createdData->copyValueFrom(data);
							createdData->setOutput(true);
							group->dataSetParent(createdData, data);
							group->addOutput(createdData);
						}

						doc->addCommand(new LinkDatasCommand(otherData, createdData));
					}
				}
			}
		}
	}

	// Select the group
	doc->addCommand(new SelectGroupCommand(doc, group));

	// Removing the objects from the document, but don't unlink datas
	doc->addCommand(new RemoveObjectCommand(doc, view, selection, false));

	return true;
}

bool Group::ungroupSelection(PandaDocument* doc, GraphView* view)
{
	if(doc->getNbSelected() < 1)
		return false;

	QList<Group*> groups;
	for(auto object : doc->getSelection())
	{
		Group* group = dynamic_cast<Group*>(object);
		if(group)
			groups.append(group);
	}

	if(groups.isEmpty())
		return false;

	auto macro = doc->beginCommandMacro(tr("ungroup selection"));

	// For each group in the selection
	for(auto group : groups)
	{
		QPointF groupPos = view->getObjectDrawStruct(group)->getPosition();

		// Putting the objects back into the document
		panda::Group::ObjectsList docks;
		for(auto object : group->getObjects())
		{
			panda::DockObject* dock = dynamic_cast<panda::DockObject*>(object.data());
			if(dock)
				docks.append(object);
			else
			{
				doc->addCommand(new RemoveObjectFromGroupCommand(group, object));
				doc->addCommand(new AddObjectCommand(doc, view, object));

				// Placing the object in the view
				ObjectDrawStruct* ods = view->getObjectDrawStruct(object.data());
				QPointF delta = groupPos + group->m_positions[object.data()] - ods->getPosition();
				if(!delta.isNull())
					doc->addCommand(new MoveObjectCommand(view, object.data(), delta));
			}
		}

		// We extract docks last (their docked objects must be out first)
		for(auto object : docks)
		{
			doc->addCommand(new RemoveObjectFromGroupCommand(group, object));
			doc->addCommand(new AddObjectCommand(doc, view, object));

			// Placing the object in the view
			ObjectDrawStruct* ods = view->getObjectDrawStruct(object.data());
			QPointF delta = groupPos + group->m_positions[object.data()] - ods->getPosition();
			if(!delta.isNull())
				doc->addCommand(new MoveObjectCommand(view, object.data(), delta));
		}

		// Reconnecting datas
		for(auto data : group->m_groupDatas)
		{
			auto parent = data->getParent();
			auto outputs = data->getOutputs();
			for(auto node : outputs)
			{
				auto outData = dynamic_cast<panda::BaseData*>(node);
				if(outData)
					doc->addCommand(new LinkDatasCommand(outData, parent));
			}
		}

		doc->addCommand(new SelectObjectsInGroupCommand(doc, group)); // Select all the object that were in the group
		doc->addCommand(new RemoveObjectCommand(doc, view, group));
	}

	return true;
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
				links.append(qMakePair(data, parent));
		}

		// Preparing dockables list for docks
		DockObject* dock = dynamic_cast<DockObject*>(object.data());
		if(dock)
		{
			for(auto dockable : dock->getDockables())
				dockedObjects.append(qMakePair(dock->getIndex(), dockable->getIndex()));
		}
	}

	// Links from objects to the output of the group
	for(BaseData* data : getOutputDatas())
	{
		BaseData* parent = data->getParent();
		if(parent)
			links.append(qMakePair(data, parent));
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
		m_groupDatas.append(dataPtr);
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

void Group::addObject(ObjectPtr obj)
{
	m_objects.push_back(obj);
}

QString Group::findAvailableDataName(QString baseName, BaseData *data)
{
	QString name = baseName;
	BaseData* testData = getData(name);
	if(testData && testData != data)
	{
		int i=2;
		testData = getData(name + QString::number(i));
		while(testData && testData != data)
		{
			++i;
			testData = getData(name + QString::number(i));
		}
		name = name + QString::number(i);
	}
	return name;
}

BaseData* Group::duplicateData(BaseData* data)
{
	if(!data)
		return nullptr;

	QString name = findAvailableDataName(data->getName());

	QSharedPointer<BaseData> newData = QSharedPointer<BaseData>(
		DataFactory::getInstance()->create(data->getDataTrait()->fullTypeId(),
										   name, data->getHelp(), this) );
	newData->setDisplayed(data->isDisplayed());
	newData->setPersistent(data->isPersistent());
	newData->setWidget(data->getWidget());
	m_groupDatas.append(newData);

	return newData.data();
}

void Group::reset()
{
	for(auto object : m_objects)
		object->reset();
}

QString Group::getGroupName()
{
	return m_groupName.getValue();
}

const Group::ObjectsList& Group::getObjects() const
{
	return m_objects;
}

int GroupClass = RegisterObject<Group>("Group").setDescription("Groups many object into a single one").setHidden(true);

//*************************************************************************//

GroupWithLayer::GroupWithLayer(PandaDocument* parent)
	: Group(parent)
	, m_layer(nullptr)
	, m_image(initData(&m_image, "image", "Image created by the renderers connected to this layer"))
	, m_compositionMode(0)
	, m_opacity(1.0)
{
	addOutput((DataNode*)parent);
}

void GroupWithLayer::setLayer(Layer* newLayer)
{
	m_layer = newLayer;
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
	{
		RenderersList renderers;
		for(auto object : m_objects)
		{
			Renderer* renderer = dynamic_cast<Renderer*>(object.data());
			if(renderer)
				renderers.push_back(renderer);
		}
		return renderers;
	}
}

QString GroupWithLayer::getLayerName() const
{
	if(m_layer)
		return m_layer->getLayerName();
	else
		return m_groupName.getValue();
}

void GroupWithLayer::setLayerName(QString name)
{
	if(m_layer)
		m_layer->setLayerName(name);
	else
		m_groupName.setValue(name);
}

int GroupWithLayer::getCompositionMode() const
{
	if(m_layer)
		return m_layer->getCompositionMode();
	else
		return m_compositionMode;
}

void GroupWithLayer::setCompositionMode(int mode)
{
	if(m_layer)
		m_layer->setCompositionMode(mode);
	else
	{
		if(mode != m_compositionMode)
		{
			m_compositionMode = mode;
			setDirtyValue(this);
		}
	}
}

PReal GroupWithLayer::getOpacity() const
{
	if(m_layer)
		return m_layer->getOpacity();
	else
		return m_opacity;
}

void GroupWithLayer::setOpacity(PReal opa)
{
	if(m_layer)
		m_layer->setOpacity(opa);
	else
	{
		if(opa != m_opacity)
		{
			m_opacity = opa;
			setDirtyValue(this);
		}
	}
}

Data<types::ImageWrapper>* GroupWithLayer::getImage()
{
	if(m_layer)
		return m_layer->getImage();
	else
		return &m_image;
}

QMatrix4x4& GroupWithLayer::getMVPMatrix()
{
	if(m_layer)
		return m_layer->getMVPMatrix();
	else
		return m_mvpMatrix;
}

void GroupWithLayer::addObject(ObjectPtr obj)
{
	Group::addObject(obj);

	Layer* layer = dynamic_cast<Layer*>(obj.data());
	if(layer)
	{
		setLayer(layer);
		return;
	}

	Layer* defaultLayer = m_parentDocument->getDefaultLayer();
	Renderer* renderer = dynamic_cast<Renderer*>(obj.data());
	if(renderer)
	{
		if(renderer->getParentDock() == defaultLayer)
			defaultLayer->removeDockable(renderer);
		renderer->setParentDock(nullptr);
		addInput((DataNode*)renderer);
	}
}

void GroupWithLayer::removeObject(PandaObject* obj)
{
	Group::removeObject(obj);

	Renderer* renderer = dynamic_cast<Renderer*>(obj);
	if(renderer && !renderer->getParentDock())
		m_parentDocument->getDefaultLayer()->addDockable(renderer);
}

int GroupWithLayerClass = RegisterObject<GroupWithLayer>("GroupWithLayer").setDescription("Groups many object into a single one (version with a layer)").setHidden(true);

} // namespace panda
