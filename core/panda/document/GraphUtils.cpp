#include <panda/document/GraphUtils.h>
#include <panda/object/Group.h>
#include <panda/helper/algorithm.h>

#include <deque>
#include <set>

namespace panda 
{

namespace graph 
{

std::vector<PandaObject*> getRawObjectsList(std::vector<std::shared_ptr<PandaObject>> objects)
{
	std::vector<PandaObject*> result;
	result.reserve(objects.size());
	for (const auto& sptr : objects)
		result.push_back(sptr.get());

	return result;
}

// The objects are given by value so that we can directly modify the list
std::vector<PandaObject*> expandObjectsList(std::vector<PandaObject*> objects)
{
	unsigned int i=0;
	while(i < objects.size())
	{
		Group* group = dynamic_cast<Group*>(objects[i]);
		if(group)
		{
			helper::removeAt(objects, i);
			for(auto& object : group->getObjectsList().get())
				objects.push_back(object.get());
		}
		else
			++i;
	}
	return objects;
}

std::vector<PandaObject*> computeConnectedObjects(const std::vector<PandaObject*>& objects)
{
	std::set<PandaObject*> closedList;
	std::deque<PandaObject*> openList;
	openList.assign(objects.begin(), objects.end());
	while(!openList.empty())
	{
		PandaObject* object = openList.front();
		openList.pop_front();
		closedList.insert(object);

		for(BaseData* data : object->getInputDatas())
		{
			if(data->getParent())
			{
				PandaObject* connected = data->getParent()->getOwner();
				if(!closedList.count(connected))
					openList.push_back(connected);
			}
		}

		for(BaseData* data : object->getOutputDatas())
		{
			for(DataNode* otherNode : data->getOutputs())
			{
				BaseData* otherData = dynamic_cast<BaseData*>(otherNode);
				if(otherData)
				{
					PandaObject* connected = otherData->getOwner();
					if (!closedList.count(connected))
						openList.push_back(connected);
				}
			}
		}

		DockableObject* dockable = dynamic_cast<DockableObject*>(object);
		if(dockable)
		{
			PandaObject* dock = dockable->getParentDock();
			if (dock != dockable->getDefaultDock() && !closedList.count(dock))
				openList.push_back(dock);
		}

		DockObject* dock = dynamic_cast<DockObject*>(object);
		if(dock)
		{
			for(auto dockable : dock->getDockedObjects())
			{
				if (!closedList.count(dockable))
					openList.push_back(dockable);
			}
		}
	}

	return std::vector<PandaObject*>(closedList.begin(), closedList.end());
}

void forEachObjectOutput(PandaObject* startObject, ObjectFunctor func)
{
	for(auto output : startObject->getOutputs())
	{
		PandaObject* object = dynamic_cast<PandaObject*>(output);
		BaseData* data = dynamic_cast<BaseData*>(output);
		if(object) // Some objects can be directly connected to others objects (Docks and Dockable for example)
		{
			func(object);
		}
		else if(data)
		{
			for(auto node : data->getOutputs())
			{
				PandaObject* object2 = dynamic_cast<PandaObject*>(node);
				BaseData* data2 = dynamic_cast<BaseData*>(node);
				if(object2)
				{ // Output data directly connected to another object
					func(object2);
				}
				else if(data2)
				{
					if(data2->getOwner() && !dynamic_cast<Group*>(data2->getOwner()))
					{ // Most objects' data are connected to another object's data
						func(data2->getOwner());
					}
					else
					{ // Groups can have inside object's data connected to the group's data, connected to outside object's data.
						for(auto node2 : data2->getOutputs())
						{
							BaseData* data3 = dynamic_cast<BaseData*>(node2);
							if(data3 && data3->getOwner())
								func(data3->getOwner());
						}
					}
				}
			}
		}
	}
}

void forEachObjectInput(PandaObject* startObject, ObjectFunctor func)
{
	for(auto output : startObject->getInputs())
	{
		PandaObject* object = dynamic_cast<PandaObject*>(output);
		BaseData* data = dynamic_cast<BaseData*>(output);
		if(object) // Some objects can be directly connected to others objects (Docks and Dockable for example)
		{
			func(object);
		}
		else if(data)
		{
			for(auto node : data->getInputs())
			{
				PandaObject* object2 = dynamic_cast<PandaObject*>(node);
				BaseData* data2 = dynamic_cast<BaseData*>(node);
				if(object2)
				{ // Output data directly connected to another object
					func(object2);
				}
				else if(data2)
				{
					if(data2->getOwner() && !dynamic_cast<Group*>(data2->getOwner()))
					{ // Most objects' data are connected to another object's data
						func(data2->getOwner());
					}
					else
					{ // Groups can have inside object's data connected to the group's data, connected to outside object's data.
						for(auto node2 : data2->getInputs())
						{
							BaseData* data3 = dynamic_cast<BaseData*>(node2);
							if(data3 && data3->getOwner())
								func(data3->getOwner());
						}
					}
				}
			}
		}
	}
}

std::vector<DataNode*> computeConnectedInputNodes(DataNode* originNode, bool keepRecursive)
{
	std::set<DataNode*> closedList;
	std::deque<DataNode*> openList;
	openList.push_back(originNode);
	while(!openList.empty())
	{
		auto* currentNode = openList.front();
		openList.pop_front();
		closedList.insert(currentNode);

		std::vector<DataNode*> inputs;
		if(keepRecursive)
			inputs = currentNode->getInputs();
		else
		{
			auto object = dynamic_cast<PandaObject*>(currentNode);
			if (object)
				inputs = object->getNonRecursiveInputs();
			else
				inputs = currentNode->getInputs();
		}
		
		for(auto node : inputs)
		{
			if(!closedList.count(node))
				openList.push_back(node);
		}
	}

	return std::vector<DataNode*>(closedList.begin(), closedList.end());
}

std::vector<DataNode*> computeConnectedOutputNodes(DataNode* originNode, bool keepRecursive)
{
	std::set<DataNode*> closedList;
	std::deque<DataNode*> openList;
	openList.push_back(originNode);
	while(!openList.empty())
	{
		auto* currentNode = openList.front();
		openList.pop_front();
		closedList.insert(currentNode);

		std::vector<DataNode*> outputs;
		if(keepRecursive)
			outputs = currentNode->getOutputs();
		else
		{
			auto object = dynamic_cast<PandaObject*>(currentNode);
			if (object)
				outputs = object->getNonRecursiveOutputs();
			else
				outputs = currentNode->getOutputs();
		}
		
		for(auto node : outputs)
		{
			if(!closedList.count(node))
				openList.push_back(node);
		}
	}

	return std::vector<DataNode*>(closedList.begin(), closedList.end());
}

std::vector<BaseData*> extractDatas(const std::vector<DataNode*>& nodes)
{
	std::vector<BaseData*> datas;
	for (const auto node : nodes)
	{
		auto data = dynamic_cast<BaseData*>(node);
		if (data)
			datas.push_back(data);
	}

	return datas;
}

std::vector<PandaObject*> extractObjects(const std::vector<DataNode*>& nodes)
{
	std::vector<PandaObject*> objects;
	for (const auto node : nodes)
	{
		auto object = dynamic_cast<PandaObject*>(node);
		if (object)
			objects.push_back(object);
	}

	return objects;
}

} // namespace graph

} // namespace panda
