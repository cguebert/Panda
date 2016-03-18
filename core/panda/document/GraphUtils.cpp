#include <panda/document/GraphUtils.h>
#include <panda/object/Group.h>
#include <panda/helper/algorithm.h>

#include <set>

namespace panda 
{

namespace graph 
{

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
			for(auto& object : group->getObjects())
				objects.push_back(object.get());
		}
		else
			++i;
	}
	return objects;
}

std::vector<PandaObject*> computeConnectedObjects(const std::vector<PandaObject*>& objects)
{
	std::set<PandaObject*> closedList, openList;
	openList.insert(objects.begin(), objects.end());
	while(!openList.empty())
	{
		PandaObject* object = *openList.begin();
		openList.erase(object);
		closedList.insert(object);

		for(BaseData* data : object->getInputDatas())
		{
			if(data->getParent())
			{
				PandaObject* connected = data->getParent()->getOwner();
				if(!closedList.count(connected))
					openList.insert(connected);
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
						openList.insert(connected);
				}
			}
		}

		DockableObject* dockable = dynamic_cast<DockableObject*>(object);
		if(dockable)
		{
			PandaObject* dock = dockable->getParentDock();
			if (dock != dockable->getDefaultDock() && !closedList.count(dock))
				openList.insert(dock);
		}

		DockObject* dock = dynamic_cast<DockObject*>(object);
		if(dock)
		{
			for(auto dockable : dock->getDockedObjects())
			{
				if (!closedList.count(dockable))
					openList.insert(dockable);
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


} // namespace graph

} // namespace panda
