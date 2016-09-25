#include <panda/document/Serialization.h>

#include <panda/document/InteractiveDocument.h>
#include <panda/SimpleGUI.h>
#include <panda/XmlDocument.h>
#include <panda/document/DocumentSignals.h>
#include <panda/document/ObjectsList.h>
#include <panda/helper/algorithm.h>
#include <panda/object/Dockable.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/ObjectAddons.h>

namespace
{

	panda::BaseData* findData(const panda::ObjectsList& objects, uint32_t objectIndex, const std::string& dataName)
	{
		auto object = objects.find(objectIndex);
		if(object)
			return object->getData(dataName);

		return nullptr;
	}

}

namespace panda 
{

namespace serialization 
{

bool writeFile(PandaDocument* document, const std::string& fileName)
{
	XmlDocument doc;
	auto root = doc.root();
	root.setName("Panda");
	document->save(root);	// The document's Datas

	ObjectAddonsRegistry::instance().save(root); // The definition of object addons

	// Get the objects list
	auto& objects = document->getObjectsList().get();
	Objects allObjects;
	for(auto object : objects)
		allObjects.push_back(object.get());

	saveDoc(document, root, allObjects);	// The document and all of its objects

	bool result = doc.saveToFile(fileName);
	if (!result)
		document->getGUI().messageBox(gui::MessageBoxType::warning, "Panda", "Cannot write file " + fileName);

	return result;
}

std::unique_ptr<PandaDocument> readFile(const std::string& fileName, panda::gui::BaseGUI& gui)
{
	XmlDocument doc;
	if (!doc.loadFromFile(fileName))
	{
		gui.messageBox(gui::MessageBoxType::warning, "Panda", "Cannot parse xml file  " + fileName + ".");
		return nullptr;
	}

	auto root = doc.root();

	auto& objectsAddonsReg = ObjectAddonsRegistry::instance();
	objectsAddonsReg.clearDefinitions(); // Remove previous definitions
	objectsAddonsReg.load(root); // Load the definition of object addons

	auto document = std::make_unique<InteractiveDocument>(gui);

	if (loadDoc(document.get(), document->getObjectsList(), root).first)
		return document;
	else 
		return nullptr;
}

LoadResult importFile(PandaDocument* document, ObjectsList& objectsList, const std::string& fileName)
{
	XmlDocument doc;
	if (!doc.loadFromFile(fileName))
	{
		document->getGUI().messageBox(gui::MessageBoxType::warning, "Panda", "Cannot parse xml file  " + fileName + ".");
		return { false, {} };
	}

	auto root = doc.root();

	ObjectAddonsRegistry::instance().load(root); // The definition of object addons, without clearing them first

	return loadDoc(document, objectsList, root);	// All the document's objects
}

std::string writeTextDocument(PandaDocument* document, const Objects& objects)
{
	XmlDocument doc;
	auto root = doc.root();
	root.setName("Panda");

	// Here we do not save the document's Data, only the objects from the list
	saveDoc(document, root, objects);

	return doc.saveToMemory();
}

LoadResult readTextDocument(PandaDocument* document, ObjectsList& objectsList, const std::string& text)
{
	XmlDocument doc;
	if (!doc.loadFromMemory(text))
		return { false, {} };

	return loadDoc(document, objectsList, doc.root());
}

bool saveDoc(PandaDocument* document, XmlElement& root, const Objects& objects)
{
	typedef std::pair<BaseData*, BaseData*> DataPair;
	std::vector<DataPair> links;

	typedef std::pair<uint32_t, uint32_t> IntPair;
	std::vector<IntPair> dockedObjects;

	// Saving objects
	for(auto object : objects)
	{
		auto elem = root.addChild("Object");
		elem.setAttribute("type", ObjectFactory::getRegistryName(object));
		elem.setAttribute("index", object->getIndex());

		object->save(elem, &objects);
		object->addons().save(elem);

		// Preparing links
		for(BaseData* data : object->getInputDatas())
		{
			BaseData* parent = data->getParent();
			if(parent && helper::contains(objects, parent->getOwner()))
				links.push_back(std::make_pair(data, parent));
		}

		// Preparing dockables list for docks
		DockObject* dock = dynamic_cast<DockObject*>(object);
		if(dock)
		{
			for(auto dockable : dock->getDockedObjects())
				dockedObjects.push_back(std::make_pair(dock->getIndex(), dockable->getIndex()));
		}
	}

	// Saving links
	for(const auto& link : links)
	{
		auto elem = root.addChild("Link");
		elem.setAttribute("object1", link.first->getOwner()->getIndex());
		elem.setAttribute("data1", link.first->getName());
		elem.setAttribute("object2", link.second->getOwner()->getIndex());
		elem.setAttribute("data2", link.second->getName());
	}

	// Saving docked objects list
	for(const auto& dockable : dockedObjects)
	{
		auto elem = root.addChild("Dock");
		elem.setAttribute("dock", dockable.first);
		elem.setAttribute("docked", dockable.second);
	}

	return true;
}

LoadResult loadDoc(PandaDocument* document, ObjectsList& objectsList, const XmlElement& root)
{
	document->getSignals().startLoading.run();
	std::map<uint32_t, uint32_t> importIndicesMap;
	auto factory = ObjectFactory::getInstance();

	using ObjectXmlPair = std::pair<std::shared_ptr<PandaObject>, XmlElement>;
	std::vector<ObjectXmlPair> newObjects;

	// Loading objects
	for(auto elem = root.firstChild("Object"); elem; elem = elem.nextSibling("Object"))
	{
		std::string registryName = elem.attribute("type").toString();
		if(registryName.empty())
			return { false, {} };
		uint32_t index = elem.attribute("index").toUnsigned();
		auto object = factory->create(registryName, document);
		if(object)
		{
			importIndicesMap[index] = object->getIndex();

			if (!object->load(elem))
				return { false, {} };
			object->addons().load(elem);

			newObjects.emplace_back(object, elem);
		}
		else
		{
			document->getGUI().messageBox(gui::MessageBoxType::warning, "Panda", "Could not create the object " + registryName + ".\nA plugin must be missing.");
			return { false, {} };
		}
	}

	// Now that we have created all the objects, we actually add them to the document
	Objects objects;
	for (const auto& p : newObjects)
	{
		const auto& object = p.first;
		objectsList.addObject(object);
		objects.push_back(object.get());
	}

	// Create links
	for(auto elem = root.firstChild("Link"); elem; elem = elem.nextSibling("Link"))
	{
		uint32_t index1, index2;
		std::string name1, name2;
		index1 = elem.attribute("object1").toUnsigned();
		index2 = elem.attribute("object2").toUnsigned();
		index1 = importIndicesMap[index1];
		index2 = importIndicesMap[index2];

		name1 = elem.attribute("data1").toString();
		name2 = elem.attribute("data2").toString();

		BaseData *data1, *data2;
		data1 = findData(document->getObjectsList(), index1, name1);
		data2 = findData(document->getObjectsList(), index2, name2);
		if(data1 && data2)
			data1->setParent(data2);
	}

	// Put dockables in their docks
	for(auto elem = root.firstChild("Dock"); elem; elem = elem.nextSibling("Dock"))
	{
		uint32_t dockIndex, dockableIndex;
		dockIndex = elem.attribute("dock").toUnsigned();
		dockableIndex = elem.attribute("docked").toUnsigned();
		dockIndex = importIndicesMap[dockIndex];
		dockableIndex = importIndicesMap[dockableIndex];

		DockObject* dock = dynamic_cast<DockObject*>(objectsList.find(dockIndex));
		DockableObject* dockable = dynamic_cast<DockableObject*>(objectsList.find(dockableIndex));
		if(dock && dockable)
		{
			DockObject* defaultDock = dockable->getDefaultDock();
			if(defaultDock)
				defaultDock->removeDockable(dockable);
			dock->addDockable(dockable);
		}
	}

	// Reset all the objects we loaded
	for(auto object : objects)
		object->reset();

	document->getSignals().loadingFinished.run(); // For example if the view wants to do some computation

	return { true, objects };
}

} // namespace serialization

} // namespace panda
