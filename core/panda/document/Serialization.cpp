#include <panda/document/Serialization.h>

#include <panda/document/InteractiveDocument.h>
#include <panda/SimpleGUI.h>
#include <panda/XmlDocument.h>
#include <panda/document/DocumentSignals.h>
#include <panda/document/ObjectsList.h>
#include <panda/helper/algorithm.h>
#include <panda/helper/Exception.h>
#include <panda/object/Dockable.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/ObjectAddons.h>
#include <panda/object/visualizer/VisualizerDocument.h>

namespace
{

	panda::BaseData* findData(const panda::ObjectsList& objects, uint32_t objectIndex, const std::string& dataName)
	{
		auto object = objects.find(objectIndex);
		if(object)
			return object->getData(dataName);

		return nullptr;
	}

	using DocumentTypes = std::vector<std::pair<panda::serialization::DocumentType, std::string>>;
	const DocumentTypes& getDocumentTypes()
	{
		static DocumentTypes types = []	{
			using DT = panda::serialization::DocumentType;
			DocumentTypes tmp;
			tmp.emplace_back(DT::Base,        "Base");
			tmp.emplace_back(DT::Rendered,    "Rendered");
			tmp.emplace_back(DT::Interactive, "Interactive");
			tmp.emplace_back(DT::Visualizer,  "Visualizer");
			return tmp;
		}();
		return types;
	}

	panda::serialization::DocumentType getDocumentType(const panda::XmlElement& root)
	{
		auto node = root.firstChild("Document");
		if(!node)
			return panda::serialization::DocumentType::Interactive;
		const auto str = node.attribute("type").toString();
		return panda::serialization::getDocumentType(str);
	}

	void writeDocumentType(panda::PandaDocument* document, panda::XmlElement& root)
	{
		const auto type = panda::serialization::getDocumentType(document);
		const auto str = panda::serialization::getDocumentName(type);
		auto node = root.addChild("Document");
		node.setAttribute("type", str);
	}

	std::unique_ptr<panda::PandaDocument> createDocument(const panda::XmlElement& root, panda::gui::BaseGUI& gui)
	{
		const auto type = getDocumentType(root);
		return panda::serialization::createDocument(type, gui);
	}

	bool canImport(panda::PandaDocument* document, const panda::XmlElement& root)
	{
		const auto currentType = panda::serialization::getDocumentType(document);
		const auto importType = getDocumentType(root);
		return canImport(currentType, importType);
	}

}

namespace panda 
{

namespace serialization 
{

std::string getDocumentName(DocumentType type)
{
	const auto index = static_cast<int>(type);
	return getDocumentTypes()[index].second;
}

DocumentType getDocumentType(const std::string& name)
{
	const auto& list = getDocumentTypes();
	auto it = std::find_if(list.begin(), list.end(), [&name](const auto& p) {
		return p.second == name;
	});
	if (it != list.end())
		return it->first;
	return DocumentType::Base;
}

DocumentType getDocumentType(panda::PandaDocument* document)
{
	if (dynamic_cast<panda::InteractiveDocument*>(document))
		return DocumentType::Interactive;
	else if (dynamic_cast<panda::VisualizerDocument*>(document))
		return DocumentType::Visualizer;
	else if (dynamic_cast<panda::RenderedDocument*>(document))
		return DocumentType::Rendered;
	return DocumentType::Base;
}

std::unique_ptr<panda::PandaDocument> createDocument(DocumentType type, panda::gui::BaseGUI& gui)
{
	switch (type)
	{
	case DocumentType::Base:        return std::make_unique<panda::PandaDocument>(gui);
	case DocumentType::Rendered:    return std::make_unique<panda::RenderedDocument>(gui);
	case DocumentType::Interactive: return std::make_unique<panda::InteractiveDocument>(gui);
	case DocumentType::Visualizer:  return std::make_unique<panda::VisualizerDocument>(gui);
	}

	throw std::exception("Unknown document type in createDocument");
}

bool canImport(DocumentType current, DocumentType import)
{
	using DT = DocumentType;
	switch (current)
	{
	case DT::Base:
		return import == DT::Base;
	case DT::Rendered:
		return import == DT::Base 
			|| import == DT::Rendered;
	case DT::Interactive:
		return import == DT::Base 
			|| import == DT::Rendered 
			|| import == DT::Interactive;
	}

	return false;
}

bool writeFile(PandaDocument* document, const std::string& fileName)
{
	XmlDocument doc;
	auto root = doc.root();
	root.setName("Panda");
	writeDocumentType(document, root);
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
		throw helper::Exception("Cannot write file: " + fileName);

	return result;
}

std::unique_ptr<PandaDocument> readFile(const std::string& fileName, panda::gui::BaseGUI& gui)
{
	XmlDocument doc;
	if (!doc.loadFromFile(fileName))
		throw helper::Exception("Cannot parse xml file: " + fileName);

	auto root = doc.root();

	auto& objectsAddonsReg = ObjectAddonsRegistry::instance();
	objectsAddonsReg.clearDefinitions(); // Remove previous definitions
	objectsAddonsReg.load(root); // Load the definition of object addons

	auto document = ::createDocument(root, gui);
	document->load(root); // Load the document's Datas

	loadDoc(document.get(), document->getObjectsList(), root);
	return document;
}

Objects importFile(PandaDocument* document, ObjectsList& objectsList, const std::string& fileName)
{
	XmlDocument doc;
	if (!doc.loadFromFile(fileName))
		throw helper::Exception("Cannot parse xml file: " + fileName);

	auto root = doc.root();
	if (!::canImport(document, root))
		throw helper::Exception("Cannot import into this document, types incompatible");

	ObjectAddonsRegistry::instance().load(root); // The definition of object addons, without clearing them first

	return loadDoc(document, objectsList, root);	// All the document's objects
}

std::string writeTextDocument(PandaDocument* document, const Objects& objects)
{
	XmlDocument doc;
	auto root = doc.root();
	root.setName("Panda");

	// We save the document's type, in order to test it when importing in another document
	writeDocumentType(document, root);

	// Here we do not save the document's Data, only the objects from the list
	saveDoc(document, root, objects);

	return doc.saveToMemory();
}

Objects readTextDocument(PandaDocument* document, ObjectsList& objectsList, const std::string& text)
{
	XmlDocument doc;
	if (!doc.loadFromMemory(text))
		return { false, {} };

	auto root = doc.root();
	if (!::canImport(document, root))
		throw helper::Exception("Cannot import into this document, types incompatible");

	return loadDoc(document, objectsList, root);
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
		elem.setAttribute("type", ObjectFactory::registryName(object));
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

Objects loadDoc(PandaDocument* document, ObjectsList& objectsList, const XmlElement& root)
{
	document->getSignals().startLoading.run();
	std::map<uint32_t, uint32_t> importIndicesMap;

	using ObjectXmlPair = std::pair<std::shared_ptr<PandaObject>, XmlElement>;
	std::vector<ObjectXmlPair> newObjects;

	// Loading objects
	for(auto elem = root.firstChild("Object"); elem; elem = elem.nextSibling("Object"))
	{
		std::string registryName = elem.attribute("type").toString();
		uint32_t index = elem.attribute("index").toUnsigned();
		auto object = ObjectFactory::create(registryName, document);
		if(object)
		{
			importIndicesMap[index] = object->getIndex();

			object->load(elem);
			object->addons().load(elem);

			newObjects.emplace_back(object, elem);
		}
		else
			throw helper::Exception("Could not create the object " + registryName + ".\nA plugin must be missing.");
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

	return objects;
}

} // namespace serialization

} // namespace panda
