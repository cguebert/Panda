#include "Group.h"

#include <ui/GraphView.h>
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
	, groupName(initData(&groupName, QString("Group"), "name", "Name to be displayed for this group"))
{
}

Group::~Group()
{
}

bool Group::createGroup(PandaDocument* doc, GraphView* view)
{
	if(doc->getNbSelected() < 2)
		return false;

	PandaDocument::ObjectsList selection = doc->getSelection();
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

	if(layer == doc->getDefaultLayer())	// Won't be added in the group!
		layer = nullptr;

	Group* group = nullptr;
	if(hasRenderer)
	{
		GroupWithLayer* groupWithLayer = dynamic_cast<GroupWithLayer*>(doc->createObject(ObjectFactory::getRegistryName<GroupWithLayer>()));
		if(groupWithLayer)
			groupWithLayer->setLayer(layer);
		group = groupWithLayer;
	}
	else
		group = dynamic_cast<Group*>(doc->createObject(ObjectFactory::getRegistryName<Group>()));
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
	QMap<BaseData*, BaseData*> connectedInputDatas, connectedOutputDatas;

	// Adding the objects
	for(auto object : selection)
	{
		group->addObject(object);
		object->setParent(group);

		// Storing the position of this object in respect to the group object
		QPointF delta = view->getObjectDrawStruct(object)->getPosition() - groupPos;
		group->positions[object] = delta;

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
						group->dataSetParent(createdData, otherData);
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
						data->getOwner()->dataSetParent(data, createdData);
				}
			}
		}

		// Adding ouput datas
		for(BaseData* data : object->getOutputDatas())
		{
			for(DataNode* otherNode : data->getOutputs())
			{
				BaseData* otherData = dynamic_cast<BaseData*>(otherNode);
				if(otherData)
				{
					PandaObject* connected = otherData->getOwner();
					if(connected && !doc->isSelected(connected) && connected!=doc)
					{
						BaseData* createdData = nullptr;
						if(!connectedOutputDatas.contains(data))
						{
							createdData = group->duplicateData(data);
							createdData->copyValueFrom(data);
							group->dataSetParent(createdData, data);
							group->addOutput(createdData);
							connectedOutputDatas.insert(data, createdData);
						}
						else
							createdData = connectedOutputDatas.value(data);

						if(createdData)
							otherData->getOwner()->dataSetParent(otherData, createdData);
					}
				}
			}
		}
	}

	// Select the group
	doc->selectNone();
	doc->setCurrentSelectedObject(group);

	// Removing the objects from the document
	for(auto object : selection)
		doc->doRemoveObject(object, false); // Do not delete it

	view->modifiedObject(group);
	view->updateLinkTags();

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

	doc->selectNone();

	// For each group in the selection
	QListIterator<Group*> iter(groups);
	while(iter.hasNext())
	{
		Group* group = iter.next();
		QPointF groupPos = view->getObjectDrawStruct(group)->getPosition();

		// Putting the objects back into the document
		QList<DockObject*> docks;
		for(auto object : group->objects)
		{
			DockObject* dock = dynamic_cast<DockObject*>(object);
			if(dock)
				docks.append(dock);
			else
			{
				group->removeObject(object);
				doc->doAddObject(object);
				doc->selectionAdd(object);
				object->setParent(doc);

				// Placing the object in the view
				ObjectDrawStruct* ods = view->getObjectDrawStruct(object);
				ods->move(groupPos + group->positions[object] - ods->getPosition());
			}
		}

		// We extract docks last (their docked objects must be out first)
		QListIterator<DockObject*> dockIter = QListIterator<DockObject*>(docks);
		while(dockIter.hasNext())
		{
			PandaObject* object = dockIter.next();
			group->removeObject(object);
			doc->doAddObject(object);
			doc->selectionAdd(object);
			object->setParent(doc);

			// Placing the object in the view
			ObjectDrawStruct* ods = view->getObjectDrawStruct(object);
			ods->move(groupPos + group->positions[object] - ods->getPosition());
		}

		// Reconnecting datas
		for(QSharedPointer<BaseData> data : group->groupDatas)
		{
			BaseData* parent = data->getParent();
			for(DataNode* node : data->getOutputs())
			{
				BaseData* outData = dynamic_cast<BaseData*>(node);
				if(outData)
					outData->getOwner()->dataSetParent(outData, parent);
			}
		}

		doc->doRemoveObject(group);
	}

	view->updateLinkTags();

	return true;
}

void Group::save(QDomDocument& doc, QDomElement& elem, const QList<PandaObject*>* selected)
{
	// Saving group datas
	for(QSharedPointer<BaseData> data : groupDatas)
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

	QList<PandaObject*> allObjects(objects);
	allObjects.push_back(this);

	// Saving objects in this group
	for(PandaObject* object : objects)
	{
		QDomElement node = doc.createElement("Object");
		node.setAttribute("type", ObjectFactory::getRegistryName(object));
		node.setAttribute("index", object->getIndex());
		elem.appendChild(node);

		object->save(doc, node, &allObjects);

		QPointF pos = positions[object];
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
		DockObject* dock = dynamic_cast<DockObject*>(object);
		if(dock)
		{
			DockObject::DockablesIterator dockableIter = dock->getDockablesIterator();
			while(dockableIter.hasNext())
				dockedObjects.append(qMakePair(dock->getIndex(), dockableIter.next()->getIndex()));
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

		BaseData* data = DataFactory::getInstance()->create(type, name, help, this);
		groupDatas.append( QSharedPointer<BaseData>(data) );
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
	PandaDocument* doc = dynamic_cast<PandaDocument*>(parent());

	QDomElement objectNode = elem.firstChildElement("Object");
	while(!objectNode.isNull())
	{
		QString registryName = objectNode.attribute("type");
		quint32 index = objectNode.attribute("index").toUInt();
		PandaObject* object = factory->create(registryName, doc);
		if(object)
		{
			importObjectsMap[index] = object;
			objects.append(object);
			object->setParent(this);

			object->load(objectNode);

			QPointF pos;
			pos.setX(objectNode.attribute("x").toDouble());
			pos.setY(objectNode.attribute("y").toDouble());
			positions[object] = pos;
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

	emit modified(this);
}

void Group::addObject(PandaObject* obj)
{
	objects.append(obj);
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
	groupDatas.append(newData);

	return newData.data();
}

void Group::reset()
{
	for(PandaObject* obj : objects)
		obj->reset();
}

QString Group::getGroupName()
{
	return groupName.getValue();
}

int GroupClass = RegisterObject<Group>("Group").setDescription("Groups many object into a single one").setHidden(true);

//*************************************************************************//

GroupWithLayer::GroupWithLayer(PandaDocument* parent)
	: Group(parent)
	, layer(nullptr)
	, image(initData(&image, "image", "Image created by the renderers connected to this layer"))
	, compositionMode(0)
	, opacity(1.0)
{
	addOutput((DataNode*)parent);
}

void GroupWithLayer::setLayer(Layer* newLayer)
{
	layer = newLayer;
}

void GroupWithLayer::update()
{
	updateLayer(parentDocument);
	cleanDirty();
}

QList<Renderer*> GroupWithLayer::getRenderers()
{
	if(layer)
		return layer->getRenderers();
	else
	{
		QList<Renderer*> renderers;
		QListIterator<PandaObject*> iter = QListIterator<PandaObject*>(objects);
		while(iter.hasNext())
		{
			Renderer* renderer = dynamic_cast<Renderer*>(iter.next());
			if(renderer)
				renderers.append(renderer);
		}
		return renderers;
	}
}

QString GroupWithLayer::getLayerName() const
{
	if(layer)
		return layer->getLayerName();
	else
		return groupName.getValue();
}

void GroupWithLayer::setLayerName(QString name)
{
	if(layer)
		layer->setLayerName(name);
	else
		groupName.setValue(name);
}

int GroupWithLayer::getCompositionMode() const
{
	if(layer)
		return layer->getCompositionMode();
	else
		return compositionMode;
}

void GroupWithLayer::setCompositionMode(int mode)
{
	if(layer)
		layer->setCompositionMode(mode);
	else
	{
		if(mode != compositionMode)
		{
			compositionMode = mode;
			setDirtyValue();
		}
	}
}

double GroupWithLayer::getOpacity() const
{
	if(layer)
		return layer->getOpacity();
	else
		return opacity;
}

void GroupWithLayer::setOpacity(double opa)
{
	if(layer)
		layer->setOpacity(opa);
	else
	{
		if(opa != opacity)
		{
			opacity = opa;
			setDirtyValue();
		}
	}
}

Data<types::ImageWrapper>* GroupWithLayer::getImage()
{
	if(layer)
		return layer->getImage();
	else
		return &image;
}

void GroupWithLayer::addObject(PandaObject* obj)
{
	Group::addObject(obj);

	Layer* defaultLayer = parentDocument->getDefaultLayer();
	Renderer* renderer = dynamic_cast<Renderer*>(obj);
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
		parentDocument->getDefaultLayer()->addDockable(renderer);
}

int GroupWithLayerClass = RegisterObject<GroupWithLayer>("GroupWithLayer").setDescription("Groups many object into a single one (version with a layer)").setHidden(true);

} // namespace panda
