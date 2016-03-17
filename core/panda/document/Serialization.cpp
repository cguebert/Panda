#include <panda/document/Serialization.h>

#include <panda/PandaDocument.h>
#include <panda/SimpleGUI.h>
#include <panda/XmlDocument.h>
#include <panda/document/DocumentSignals.h>
#include <panda/helper/algorithm.h>
#include <panda/object/Dockable.h>
#include <panda/object/ObjectFactory.h>

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
	auto& objects = document->getObjects();
	std::vector<PandaObject*> allObjects;
	for(auto object : objects)
		allObjects.push_back(object.get());
	saveDoc(document, root, allObjects);	// The document and all of its objects

	bool result = doc.saveToFile(fileName);
	if (!result)
		document->getGUI().messageBox(gui::MessageBoxType::warning, "Panda", "Cannot write file " + fileName);

	return result;
}

bool readFile(PandaDocument* document, const std::string& fileName, bool isImport)
{
	XmlDocument doc;
	if (!doc.loadFromFile(fileName))
	{
		document->getGUI().messageBox(gui::MessageBoxType::warning, "Panda", "Cannot parse xml file  " + fileName + ".");
		return false;
	}

	auto root = doc.root();
	if(!isImport)	// Bugfix: don't read the doc's datas if we are merging 2 documents
		document->load(root);		// Only the document's Datas
	loadDoc(document, root);	// All the document's objects

	for(auto& object : document->getObjects())
		object->reset();

	document->getSignals().selectionChanged.run();
	document->getSignals().selectedObject.run(document->getCurrentSelectedObject());

	return true;
}

std::string writeTextDocument(PandaDocument* document)
{
	XmlDocument doc;
	auto root = doc.root();
	root.setName("Panda");

	saveDoc(document, root, document->getSelection());

	return doc.saveToMemory();
}

bool readTextDocument(PandaDocument* document, const std::string& text)
{
	XmlDocument doc;
	if (!doc.loadFromMemory(text))
		return false;

	bool bSelected = !document->getSelection().empty();
	bool bVal = loadDoc(document, doc.root());

	for(auto object : document->getSelection())
		object->reset();

	if(bSelected || !document->getSelection().empty())
	{
		document->getSignals().selectionChanged.run();
		document->getSignals().selectedObject.run(document->getCurrentSelectedObject());
	}

	return bVal;
}

bool saveDoc(PandaDocument* document, XmlElement& root, const std::vector<PandaObject*>& selected)
{
	typedef std::pair<BaseData*, BaseData*> DataPair;
	std::vector<DataPair> links;

	typedef std::pair<uint32_t, uint32_t> IntPair;
	std::vector<IntPair> dockedObjects;

	// Saving objects
	for(auto object : selected)
	{
		auto elem = root.addChild("Object");
		elem.setAttribute("type", ObjectFactory::getRegistryName(object));
		elem.setAttribute("index", object->getIndex());

		object->save(elem, &selected);

		// Preparing links
		for(BaseData* data : object->getInputDatas())
		{
			BaseData* parent = data->getParent();
			if(parent && helper::contains(selected, parent->getOwner()))
				links.push_back(std::make_pair(data, parent));
		}

		// Preparing dockables list for docks
		DockObject* dock = dynamic_cast<DockObject*>(object);
		if(dock)
		{
			for(auto dockable : dock->getDockedObjects())
				dockedObjects.push_back(std::make_pair(dock->getIndex(), dockable->getIndex()));
		}

		document->getSignals().savingObject.run(elem, object);
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

bool loadDoc(PandaDocument* document, XmlElement& root)
{
	std::vector<PandaObject*> selectedObjects;
	document->getSignals().startLoading.run();
	std::map<uint32_t, uint32_t> importIndicesMap;
	auto factory = ObjectFactory::getInstance();

	using ObjectXmlPair = std::pair<std::shared_ptr<PandaObject>, XmlElement>;
	std::vector<ObjectXmlPair> newObjects;

	// Loading objects
	auto elem = root.firstChild("Object");
	while(elem)
	{
		std::string registryName = elem.attribute("type").toString();
		if(registryName.empty())
			return false;
		uint32_t index = elem.attribute("index").toUnsigned();
		auto object = factory->create(registryName, document);
		if(object)
		{
			importIndicesMap[index] = object->getIndex();

			if (!object->load(elem))
				return false;

			newObjects.emplace_back(object, elem);
		}
		else
		{
			document->getGUI().messageBox(gui::MessageBoxType::warning, "Panda", "Could not create the object " + registryName + ".\nA plugin must be missing.");
			return false;
		}

		elem = elem.nextSibling("Object");
	}

	// Now that we have created all the objects, we actually add them to the document
	for (const auto& p : newObjects)
	{
		const auto& object = p.first;
		document->addObject(object);
		selectedObjects.push_back(object.get());
		document->getSignals().loadingObject.run(p.second, object.get());
	}

	document->setSelection(selectedObjects);

	// Create links
	elem = root.firstChild("Link");
	while(elem)
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
		data1 = document->findData(index1, name1);
		data2 = document->findData(index2, name2);
		if(data1 && data2)
			data1->setParent(data2);

		elem = elem.nextSibling("Link");
	}

	// Put dockables in their docks
	elem = root.firstChild("Dock");
	while(elem)
	{
		uint32_t dockIndex, dockableIndex;
		dockIndex = elem.attribute("dock").toUnsigned();
		dockableIndex = elem.attribute("docked").toUnsigned();
		dockIndex = importIndicesMap[dockIndex];
		dockableIndex = importIndicesMap[dockableIndex];

		DockObject* dock = dynamic_cast<DockObject*>(document->findObject(dockIndex));
		DockableObject* dockable = dynamic_cast<DockableObject*>(document->findObject(dockableIndex));
		if(dock && dockable)
		{
			DockObject* defaultDock = dockable->getDefaultDock();
			if(defaultDock)
				defaultDock->removeDockable(dockable);
			dock->addDockable(dockable);
		}

		elem = elem.nextSibling("Dock");
	}

	document->getSignals().loadingFinished.run(); // For example if the view wants to do some computation

	return true;
}

} // namespace serialization

} // namespace panda
