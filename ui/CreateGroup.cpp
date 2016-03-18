#include <ui/CreateGroup.h>
#include <ui/GraphView.h>
#include <ui/drawstruct/ObjectDrawStruct.h>

#include <ui/command/AddObjectCommand.h>
#include <ui/command/RemoveObjectCommand.h>
#include <ui/command/MoveObjectCommand.h>


#include <panda/PandaDocument.h>
#include <panda/data/DataFactory.h>
#include <panda/object/Group.h>
#include <panda/object/Renderer.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/Layer.h>
#include <panda/command/GroupCommand.h>
#include <panda/command/LinkDatasCommand.h>

#include <QMessageBox>

namespace
{

panda::types::Point convert(QPointF pt)
{
	return panda::types::Point(pt.x(), pt.y());
}

QPointF convert(panda::types::Point pt)
{
	return QPointF(pt.x, pt.y);
}

}

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

std::string findAvailableDataName(PandaObject* object, const std::string& baseName, BaseData* data = nullptr)
{
	auto name = baseName;
	BaseData* testData = object->getData(name);
	if(testData && testData != data)
	{
		int i=2;
		testData = object->getData(name + std::to_string(i));
		while(testData && testData != data)
		{
			++i;
			testData = object->getData(name + std::to_string(i));
		}
		name = name + std::to_string(i);
	}
	return name;
}

BaseData* duplicateData(Group* group, BaseData* data)
{
	auto name = findAvailableDataName(group, data->getName());

	auto newData = DataFactory::getInstance()->create(data->getDataTrait()->fullTypeId(),
										   name, data->getHelp(), group);
	newData->setDisplayed(data->isDisplayed());
	newData->setPersistent(data->isPersistent());
	newData->setWidget(data->getWidget());
	newData->setWidgetData(data->getWidgetData());
	group->addGroupData(newData);

	return newData.get();
}

bool createGroup(PandaDocument* doc, GraphView* view)
{
	if(doc->getSelection().size() < 2)
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
				QMessageBox::warning(nullptr, "Panda", "All renderers must be placed in the same layer.");
				return false;
			}

			if(layer && layer != doc->getDefaultLayer() && !doc->isSelected(layer))
			{
				QMessageBox::warning(nullptr, "Panda", "Renderers must be grouped with their layers");
				return false;
			}
		}
	}

	auto& undoStack = doc->getUndoStack();
	auto macro = undoStack.beginMacro("create Group");

	if(layer == doc->getDefaultLayer())	// Won't be added in the group!
		layer = nullptr;

	auto factory = ObjectFactory::getInstance();
	Group* group = nullptr;
	if(hasRenderer)
	{
		auto object = factory->create(ObjectFactory::getRegistryName<GroupWithLayer>(), doc);
		undoStack.push(std::make_shared<AddObjectCommand>(doc, view, object));
		auto groupWithLayer = dynamic_cast<GroupWithLayer*>(object.get());
		group = groupWithLayer;
	}
	else
	{
		auto object = factory->create(ObjectFactory::getRegistryName<Group>(), doc);
		undoStack.push(std::make_shared<AddObjectCommand>(doc, view, object));
		group = dynamic_cast<Group*>(object.get());
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
	std::map<BaseData*, BaseData*> connectedInputDatas;

	// To sort the created datas by the height of their parents
	typedef QPair<BaseData*, qreal> DataHeightPair;
	QVector<DataHeightPair> createdDatasHeights;

	// Adding the objects
	for(auto object : selection)
	{
		auto objectPtr = doc->getSharedPointer(object);
		if(!objectPtr)
			continue;
		undoStack.push(std::make_shared<AddObjectToGroupCommand>(group, objectPtr));

		// Storing the position of this object in respect to the group object
		QPointF delta = view->getObjectDrawStruct(object)->getPosition() - groupPos;
		group->setPosition(object, convert(delta));

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
					if(!connectedInputDatas.count(otherData))
					{
						createdData = duplicateData(group, data);
						createdData->copyValueFrom(otherData);
						group->addInput(*createdData);
						undoStack.push(std::make_shared<LinkDatasCommand>(createdData, otherData));
						connectedInputDatas.emplace(otherData, createdData);
						createdDatasHeights.push_back(qMakePair(createdData, getDataHeight(view, otherData)));
					}
					else
					{
						createdData = connectedInputDatas.at(otherData);
						auto name = findAvailableDataName(group, otherData->getName(), createdData);
						if(name != createdData->getName())
						{
							createdData->setName(name);
							createdData->setHelp(otherData->getHelp());
						}
					}

					if(createdData)
						undoStack.push(std::make_shared<LinkDatasCommand>(data, createdData));
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
							group->addOutput(*createdData);
							createdDatasHeights.push_back(qMakePair(createdData, getDataHeight(view, data)));
						}

						undoStack.push(std::make_shared<LinkDatasCommand>(otherData, createdData));
					}
				}
			}
		}

		// Looking for UserValue objects that can be used as inputs or outputs to the group
		BaseGeneratorUser* userValue = dynamic_cast<BaseGeneratorUser*>(object);
		if(userValue && !userValue->getCaption().empty())
		{
			auto caption = userValue->getCaption();
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
				group->addInput(*createdData);
				undoStack.push(std::make_shared<LinkDatasCommand>(inputData, createdData));
				createdDatasHeights.push_back(qMakePair(createdData, getDataHeight(view, inputData)));
			}

			if(userValue->hasConnectedOutput())
			{
				for(auto data : group->getGroupDatas())
				{
					if(data->getParent() == outputData)
					{
						data->setName(findAvailableDataName(group, caption, data.get()));
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
				group->addOutput(*createdData);
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
	undoStack.push(std::make_shared<SelectGroupCommand>(doc, group));

	// Removing the objects from the document, but don't unlink datas
	undoStack.push(std::make_shared<RemoveObjectCommand>(doc, view, selection, false));

	return true;
}

bool ungroupSelection(PandaDocument* doc, GraphView* view)
{
	if(doc->getSelection().empty())
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

	auto& undoStack = doc->getUndoStack();
	auto macro = undoStack.beginMacro("ungroup selection");

	// For each group in the selection
	for(auto group : groups)
	{
		QPointF groupPos = view->getObjectDrawStruct(group)->getPosition();

		// Putting the objects back into the document
		panda::Group::ObjectsList docks;
		for(auto& object : group->getObjects())
		{
			panda::DockObject* dock = dynamic_cast<panda::DockObject*>(object.get());
			if(dock)
				docks.push_back(object);
			else
			{
				undoStack.push(std::make_shared<AddObjectCommand>(doc, view, object));
				undoStack.push(std::make_shared<RemoveObjectFromGroupCommand>(group, object));

				// Placing the object in the view
				ObjectDrawStruct* ods = view->getObjectDrawStruct(object.get());
				QPointF delta = groupPos + convert(group->getPosition(object.get())) - ods->getPosition();
				if(!delta.isNull())
					undoStack.push(std::make_shared<MoveObjectCommand>(view, object.get(), delta));
			}
		}

		// We extract docks last (their docked objects must be out first)
		for(auto& object : docks)
		{
			undoStack.push(std::make_shared<AddObjectCommand>(doc, view, object));
			undoStack.push(std::make_shared<RemoveObjectFromGroupCommand>(group, object));

			// Placing the object in the view
			ObjectDrawStruct* ods = view->getObjectDrawStruct(object.get());
			QPointF delta = groupPos + convert(group->getPosition(object.get())) - ods->getPosition();
			if(!delta.isNull())
				undoStack.push(std::make_shared<MoveObjectCommand>(view, object.get(), delta));
		}

		// Reconnecting datas
		for(auto& data : group->getGroupDatas())
		{
			auto parent = data->getParent();
			auto outputs = data->getOutputs();
			for(auto node : outputs)
			{
				auto outData = dynamic_cast<panda::BaseData*>(node);
				if(outData)
					undoStack.push(std::make_shared<LinkDatasCommand>(outData, parent));
			}
		}

		undoStack.push(std::make_shared<SelectObjectsInGroupCommand>(doc, group)); // Select all the object that were in the group
		undoStack.push(std::make_shared<RemoveObjectCommand>(doc, view, group));
	}

	view->sortAllDockables();

	return true;
}

} // namespace panda
