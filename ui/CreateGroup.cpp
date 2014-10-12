#include <ui/CreateGroup.h>
#include <ui/GraphView.h>
#include <ui/drawstruct/ObjectDrawStruct.h>

#include <ui/command/AddObjectCommand.h>
#include <ui/command/RemoveObjectCommand.h>
#include <ui/command/MoveObjectCommand.h>

#include <panda/Group.h>
#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>
#include <panda/Layer.h>
#include <panda/Renderer.h>
#include <panda/DataFactory.h>
#include <panda/command/GroupCommand.h>
#include <panda/command/LinkDatasCommand.h>

#include <modules/generators/UserValue.h>

#include <QCoreApplication>
#include <QMessageBox>

namespace panda
{

qreal getDataHeight(GraphView* view, BaseData* data)
{
	auto owner = data->getOwner();
	if(!owner)
		return 0;

	auto ods = view->getObjectDrawStruct(owner);
	QRectF rect;
	if(ods->getDataRect(data, rect))
		return rect.center().y();

	return 0;
}

QString findAvailableDataName(PandaObject* object, QString baseName, BaseData *data = nullptr)
{
	QString name = baseName;
	BaseData* testData = object->getData(name);
	if(testData && testData != data)
	{
		int i=2;
		testData = object->getData(name + QString::number(i));
		while(testData && testData != data)
		{
			++i;
			testData = object->getData(name + QString::number(i));
		}
		name = name + QString::number(i);
	}
	return name;
}

BaseData* duplicateData(Group* group, BaseData* data)
{
	QString name = findAvailableDataName(group, data->getName());

	QSharedPointer<BaseData> newData = QSharedPointer<BaseData>(
		DataFactory::getInstance()->create(data->getDataTrait()->fullTypeId(),
										   name, data->getHelp(), group) );
	newData->setDisplayed(data->isDisplayed());
	newData->setPersistent(data->isPersistent());
	newData->setWidget(data->getWidget());
	group->addGroupData(newData);

	return newData.data();
}

bool createGroup(PandaDocument* doc, GraphView* view)
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
				QMessageBox::warning(nullptr, "Panda",
					QCoreApplication::translate("createGroup", "All renderers must be placed in the same layer."));
				return false;
			}

			if(layer && layer != doc->getDefaultLayer() && !doc->isSelected(layer))
			{
				QMessageBox::warning(nullptr, "Panda",
					QCoreApplication::translate("createGroup", "Renderers must be grouped with their layers"));
				return false;
			}
		}
	}

	auto macro = doc->beginCommandMacro(QCoreApplication::translate("createGroup", "Create Group"));

	if(layer == doc->getDefaultLayer())	// Won't be added in the group!
		layer = nullptr;

	auto factory = ObjectFactory::getInstance();
	Group* group = nullptr;
	if(hasRenderer)
	{
		auto object = factory->create(ObjectFactory::getRegistryName<GroupWithLayer>(), doc);
		doc->addCommand(new AddObjectCommand(doc, view, object));
		auto groupWithLayer = dynamic_cast<GroupWithLayer*>(object.data());
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

	// To sort the created datas by the height of their parents
	typedef QPair<BaseData*, qreal> DataHeightPair;
	QVector<DataHeightPair> createdDatasHeights;

	// Adding the objects
	for(auto object : selection)
	{
		auto objectPtr = doc->getSharedPointer(object);
		if(!objectPtr)
			continue;
		doc->addCommand(new AddObjectToGroupCommand(group, objectPtr));

		// Storing the position of this object in respect to the group object
		QPointF delta = view->getObjectDrawStruct(object)->getPosition() - groupPos;
		group->setPosition(object, delta);

		// Adding input datas
		for(BaseData* data : object->getInputDatas())
		{
			BaseData* otherData = data->getParent();
			if(otherData)
			{
				PandaObject* owner = otherData->getOwner();
				if(owner && !doc->isSelected(owner) && owner!=doc)
				{
					BaseData* createdData = nullptr;
					if(!connectedInputDatas.contains(otherData))
					{
						createdData = duplicateData(group, data);
						createdData->copyValueFrom(otherData);
						group->addInput(createdData);
						doc->addCommand(new LinkDatasCommand(createdData, otherData));
						connectedInputDatas.insert(otherData, createdData);
						createdDatasHeights.push_back(qMakePair(createdData, getDataHeight(view, otherData)));
					}
					else
					{
						createdData = connectedInputDatas.value(otherData);
						QString name = findAvailableDataName(group, otherData->getName(), createdData);
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
							createdData = duplicateData(group, data);
							createdData->copyValueFrom(data);
							createdData->setOutput(true);
							group->dataSetParent(createdData, data);
							group->addOutput(createdData);
							createdDatasHeights.push_back(qMakePair(createdData, getDataHeight(view, data)));
						}

						doc->addCommand(new LinkDatasCommand(otherData, createdData));
					}
				}
			}
		}

		// Looking for UserValue objects that can be used as inputs or outputs to the group
		BaseGeneratorUser* userValue = dynamic_cast<BaseGeneratorUser*>(object);
		if(userValue && !userValue->getCaption().isEmpty())
		{
			QString caption = userValue->getCaption();
			auto inputData = userValue->getInputUserData();
			auto outputData = userValue->getOutputUserData();
			if(userValue->hasConnectedInput())
			{
				auto data = inputData->getParent();
				if(data->getOwner() ==  group)
					data->setName(findAvailableDataName(group, caption, data));
			}
			else // We create a data in the group for this input
			{
				BaseData* createdData = duplicateData(group, inputData);
				createdData->copyValueFrom(inputData);
				createdData->setName(findAvailableDataName(group, caption, createdData));
				group->addInput(createdData);
				doc->addCommand(new LinkDatasCommand(inputData, createdData));
				createdDatasHeights.push_back(qMakePair(createdData, getDataHeight(view, inputData)));
			}

			if(userValue->hasConnectedOutput())
			{
				for(auto data : group->getGroupDatas())
				{
					if(data->getParent() == outputData)
					{
						data->setName(findAvailableDataName(group, caption, data.data()));
						data->setDisplayed(true);
					}
				}
			}
			else // We create a data in the group for this output
			{
				BaseData* createdData = duplicateData(group, outputData);
				createdData->copyValueFrom(outputData);
				createdData->setName(findAvailableDataName(group, caption, createdData));
				createdData->setOutput(true);
				group->dataSetParent(createdData, outputData);
				group->addOutput(createdData);
				createdDatasHeights.push_back(qMakePair(createdData, getDataHeight(view, outputData)));
			}
		}
	}

	// Sort the created datas based on their height
	std::sort(createdDatasHeights.begin(), createdDatasHeights.end(), [](const DataHeightPair& d1, const DataHeightPair& d2){
		return d1.second < d2.second;
	});
	for(const auto& dataPair : createdDatasHeights)
	{
		group->removeData(dataPair.first);
		group->addData(dataPair.first);
	}

	// Select the group
	doc->addCommand(new SelectGroupCommand(doc, group));

	// Removing the objects from the document, but don't unlink datas
	doc->addCommand(new RemoveObjectCommand(doc, view, selection, false));

	return true;
}

bool ungroupSelection(PandaDocument* doc, GraphView* view)
{
	if(doc->getNbSelected() < 1)
		return false;

	QList<Group*> groups;
	for(auto object : doc->getSelection())
	{
		Group* group = dynamic_cast<Group*>(object);
		if(group)
			groups.push_back(group);
	}

	if(groups.isEmpty())
		return false;

	auto macro = doc->beginCommandMacro(QCoreApplication::translate("createGroup", "ungroup selection"));

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
				docks.push_back(object);
			else
			{
				doc->addCommand(new AddObjectCommand(doc, view, object));
				doc->addCommand(new RemoveObjectFromGroupCommand(group, object));

				// Placing the object in the view
				ObjectDrawStruct* ods = view->getObjectDrawStruct(object.data());
				QPointF delta = groupPos + group->getPosition(object.data()) - ods->getPosition();
				if(!delta.isNull())
					doc->addCommand(new MoveObjectCommand(view, object.data(), delta));
			}
		}

		// We extract docks last (their docked objects must be out first)
		for(auto object : docks)
		{
			doc->addCommand(new AddObjectCommand(doc, view, object));
			doc->addCommand(new RemoveObjectFromGroupCommand(group, object));

			// Placing the object in the view
			ObjectDrawStruct* ods = view->getObjectDrawStruct(object.data());
			QPointF delta = groupPos + group->getPosition(object.data()) - ods->getPosition();
			if(!delta.isNull())
				doc->addCommand(new MoveObjectCommand(view, object.data(), delta));
		}

		// Reconnecting datas
		for(auto data : group->getGroupDatas())
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

	view->sortAllDockables();

	return true;
}

} // namespace panda
