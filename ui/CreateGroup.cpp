#include <ui/CreateGroup.h>
#include <ui/GraphView/GraphView.h>
#include <ui/GraphView/object/ObjectRenderer.h>

#include <ui/GraphView/LinksList.h>
#include <ui/GraphView/ObjectsSelection.h>
#include <ui/GraphView/ObjectRenderersList.h>
#include <ui/GraphView/object/ObjectPositionAddon.h>

#include <panda/document/RenderedDocument.h>
#include <panda/data/DataFactory.h>
#include <panda/object/Group.h>
#include <panda/object/Renderer.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/PandaObject.h>
#include <panda/object/Layer.h>
#include <panda/command/AddObjectCommand.h>
#include <panda/command/GroupCommand.h>
#include <panda/command/LinkDatasCommand.h>
#include <panda/command/RemoveObjectCommand.h>
#include <panda/document/ObjectsList.h>
#include <panda/helper/algorithm.h>

#include <QMessageBox>

using panda::types::Point;
using panda::types::Rect;

namespace panda
{

float getDataHeight(graphview::GraphView* view, BaseData* data)
{
	auto owner = data->getOwner();
	if(!owner)
		return 0;

	Rect rect;
	if(view->linksList().getDataRect(data, rect))
		return rect.center().y;

	return 0;
}

bool createGroup(PandaDocument* doc, graphview::GraphView* view)
{
	auto renderedDoc = dynamic_cast<RenderedDocument*>(doc);
	const auto& objectsSelection = view->selection();
	auto selection = objectsSelection.get(); // Taking a copy

	if(selection.size() < 2)
		return false;

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

			if(layer && renderedDoc && layer != renderedDoc->getDefaultLayer() && !objectsSelection.isSelected(layer))
			{
				QMessageBox::warning(nullptr, "Panda", "Renderers must be grouped with their layers");
				return false;
			}
		}
	}

	// Verify that all dockables are selected with their docks
	for (auto object : selection)
	{
		if (auto dock = dynamic_cast<DockObject*>(object))
		{
			for (const auto docked : dock->getDockedObjects())
			{
				if (!objectsSelection.isSelected(docked))
				{
					QMessageBox::warning(nullptr, "Panda", "All dockable objects must be selected with their dock");
					return false;
				}
			}
		}
		else if (auto dockable = dynamic_cast<DockableObject*>(object))
		{
			auto dock = dockable->getParentDock();
			if (!dock || dock == dockable->getDefaultDock())
				continue;
			if (!objectsSelection.isSelected(dock))
			{
				QMessageBox::warning(nullptr, "Panda", "All dockable objects must be selected with their dock");
				return false;
			}
		}
	}

	auto& undoStack = doc->getUndoStack();
	auto macro = undoStack.beginMacro("create Group");

	if(renderedDoc && layer == renderedDoc->getDefaultLayer())	// Won't be added in the group!
		layer = nullptr;

	auto& objectsList = view->objectsList();
	auto factory = ObjectFactory::getInstance();
	Group* group = nullptr;
	if(hasRenderer)
	{
		auto object = factory->create(ObjectFactory::getRegistryName<GroupWithLayer>(), doc);
		undoStack.push(std::make_shared<AddObjectCommand>(doc, objectsList, object));
		auto groupWithLayer = dynamic_cast<GroupWithLayer*>(object.get());
		group = groupWithLayer;
	}
	else
	{
		auto object = factory->create(ObjectFactory::getRegistryName<Group>(), doc);
		undoStack.push(std::make_shared<AddObjectCommand>(doc, objectsList, object));
		group = dynamic_cast<Group*>(object.get());
	}
	if(!group)
		return false;

	// Find center of the selection
	Rect totalView;
	for(auto object : selection)
		totalView |= view->objectRenderers().get(object)->getVisualArea();

	// Put the new object there
	auto* objRnd = view->objectRenderers().get(group);
	Point objSize = objRnd->getObjectSize() / 2;
	objRnd->move(totalView.center() - objRnd->getPosition() - objSize);
	Point groupPos = objRnd->getPosition();

	// If multiple outside datas are connected to the same data, merge them
	std::map<BaseData*, BaseData*> connectedInputDatas;

	// To sort the created datas by the height of their parents
	using DataHeightPair = std::pair<BaseData*, qreal>;
	std::vector<DataHeightPair> createdDatasHeights;

	// Adding the objects
	for(auto object : selection)
	{
		auto objectPtr = objectsList.getShared(object);
		if(!objectPtr)
			continue;
		undoStack.push(std::make_shared<AddObjectToGroupCommand>(group, objectPtr));

		// Adding input datas
		for(BaseData* data : object->getInputDatas())
		{
			BaseData* otherData = data->getParent();
			if(otherData)
			{
				PandaObject* owner = otherData->getOwner();
				if(owner && !objectsSelection.isSelected(owner) && owner!=doc)
				{
					BaseData* createdData = nullptr;
					if(!connectedInputDatas.count(otherData))
					{
						auto newData = group->duplicateData(data);
						group->addGroupData(newData);
						createdData = newData.get();
						createdData->copyValueFrom(otherData);
						group->addInput(*createdData);
						undoStack.push(std::make_shared<LinkDatasCommand>(createdData, otherData));
						connectedInputDatas.emplace(otherData, createdData);
						createdDatasHeights.emplace_back(createdData, getDataHeight(view, otherData));
					}
					else
					{
						createdData = connectedInputDatas.at(otherData);
						auto name = group->findAvailableDataName(otherData->getName(), createdData);
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
					if(connected && !objectsSelection.isSelected(connected) && connected!=doc)
					{
						if(!createdData)
						{
							auto newData = group->duplicateData(data);
							group->addGroupData(newData);
							createdData = newData.get();
							createdData->copyValueFrom(data);
							createdData->setOutput(true);
							group->dataSetParent(createdData, data);
							group->addOutput(*createdData);
							createdDatasHeights.emplace_back(createdData, getDataHeight(view, data));
						}

						undoStack.push(std::make_shared<LinkDatasCommand>(otherData, createdData));
					}
				}
			}
		}

		// Looking for UserValue objects that can be used as inputs or outputs to the group
		BaseGeneratorUser* userValue = dynamic_cast<BaseGeneratorUser*>(object);
		if(userValue && !userValue->getLabel().empty())
		{
			auto caption = userValue->getLabel();
			auto inputData = userValue->getInputUserData();
			auto outputData = userValue->getOutputUserData();
			if(userValue->hasConnectedInput())
			{
				auto data = inputData->getParent();
				if(data->getOwner() ==  group)
					data->setName(group->findAvailableDataName(caption, data));
			}
			else // We create a data in the group for this input
			{
				auto newData = group->duplicateData(inputData);
				group->addGroupData(newData);
				auto createdData = newData.get();
				createdData->copyValueFrom(inputData);
				createdData->setName(group->findAvailableDataName(caption, createdData));
				group->addInput(*createdData);
				undoStack.push(std::make_shared<LinkDatasCommand>(inputData, createdData));
				createdDatasHeights.emplace_back(createdData, getDataHeight(view, inputData));
			}

			if(userValue->hasConnectedOutput())
			{
				for(auto data : group->getGroupDatas())
				{
					if(data->getParent() == outputData)
					{
						data->setName(group->findAvailableDataName(caption, data.get()));
						data->setDisplayed(true);
					}
				}
			}
			else // We create a data in the group for this output
			{
				auto newData = group->duplicateData(inputData);
				group->addGroupData(newData);
				auto createdData = newData.get();
				createdData->copyValueFrom(outputData);
				createdData->setName(group->findAvailableDataName(caption, createdData));
				createdData->setOutput(true);
				group->dataSetParent(createdData, outputData);
				group->addOutput(*createdData);
				createdDatasHeights.emplace_back(createdData, getDataHeight(view, outputData));
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

	// Removing the objects from the document, but don't unlink datas
	undoStack.push(std::make_shared<RemoveObjectCommand>(doc, objectsList, selection, 
														 RemoveObjectCommand::LinkOperation::Keep, 
														 RemoveObjectCommand::ObjectOperation::None)); // We are not really removing the objects from the document

	// Select the group
	view->selection().add(group);

	return true;
}

bool ungroupSelection(PandaDocument* doc, graphview::GraphView* view)
{
	const auto& selection = view->selection().get();
	if(selection.empty())
		return false;

	QList<Group*> groups;
	for(auto object : selection)
	{
		Group* group = dynamic_cast<Group*>(object);
		if(group)
			groups.push_back(group);
	}

	if(groups.isEmpty())
		return false;

	auto& undoStack = doc->getUndoStack();
	auto macro = undoStack.beginMacro("ungroup selection");

	auto& objectsList = view->objectsList();

	// For each group in the selection
	for(auto group : groups)
	{
		// Putting the objects back into the document
		auto objects = group->getObjectsList().get(); // Need to get a copy as we modify the original while iterating over it

		// Compute the center of the group objects positions
		Rect objectsRect;
		Point defaultSize(100, 50);
		for (auto& object : objects)
			objectsRect |= Rect::fromSize(graphview::ObjectPositionAddon::getPosition(object.get()), defaultSize);
		auto groupRnd = view->objectRenderers().get(group);
		auto delta = groupRnd->getPosition() - objectsRect.center() - groupRnd->getObjectSize() / 2;

		// Moving the object from the group to the parent document
		for(auto& object : objects)
		{
			undoStack.push(std::make_shared<AddObjectCommand>(doc, objectsList, object, false));
			undoStack.push(std::make_shared<RemoveObjectFromGroupCommand>(group, object));
		}

		// Placing the object in the view
		for(auto& object : objects)
			graphview::ObjectPositionAddon::setPosition(object.get(), graphview::ObjectPositionAddon::getPosition(object.get()) + delta);

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

		undoStack.push(std::make_shared<RemoveObjectCommand>(doc, objectsList, group, 
															 RemoveObjectCommand::LinkOperation::Unlink, 
															 RemoveObjectCommand::ObjectOperation::None));

		// Select all the objects that were in the group
		auto& selectionList = view->selection();
		for(const auto& object : objects)
			selectionList.add(object.get());
	}

	view->sortAllDockables();

	return true;
}

} // namespace panda
