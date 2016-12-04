#include <panda/GroupsManager.h>

#include <panda/command/AddObjectCommand.h>
#include <panda/document/PandaDocument.h>
#include <panda/document/Serialization.h>
#include <panda/graphview/GraphView.h>
#include <panda/object/Group.h>
#include <panda/object/ObjectFactory.h>
#include <panda/helper/Exception.h>
#include <panda/helper/system/FileRepository.h>
#include <panda/XmlDocument.h>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace
{

	std::pair<std::string, panda::GroupsManager::GroupInformation> getInformation(const std::string& dirPath, const std::string& path)
	{
		const auto filePath = (fs::path(dirPath) / path).generic_string();
		const auto realPath = panda::helper::system::DataRepository.findFile(filePath);
		panda::XmlDocument doc;
		if (!doc.loadFromFile(realPath))
			return { "", {} };

		auto root = doc.root();
		auto descAtt = root.attribute("description");
		if(!descAtt)
			return { "", {} };

		const auto pPath = fs::path(path);
		const auto parentPath = pPath.parent_path();
		std::string groupPath;
		if (!parentPath.empty())
			groupPath = (parentPath / pPath.stem()).string();
		else
			groupPath = pPath.stem().string();

		auto docTypeAtt = root.attribute("document");
		auto docType = panda::serialization::DocumentType::Base;
		if (docTypeAtt)
			docType = panda::serialization::getDocumentType(docTypeAtt.toString());

		return { groupPath, { descAtt.toString(), docType } };
	}

}

namespace panda
{

	const std::string GroupsManager::m_dirPath = "groups";

	GroupsManager& GroupsManager::instance()
	{
		static GroupsManager groupManager;
		return groupManager;
	}

	void GroupsManager::createGroupsList()
	{
		auto& vm = instance();
		
		auto& map = vm.m_groupsMap;
		map.clear();

		const auto paths = panda::helper::system::DataRepository.enumerateFilesInDir(m_dirPath, ".grp");
		for (auto&& path : paths)
		{
			const auto infoPair = getInformation(m_dirPath, path);
			if (!infoPair.first.empty())
				map[infoPair.first] = std::move(infoPair.second);
		}
	}

	bool GroupsManager::canCreate(const std::string& groupPath, panda::serialization::DocumentType docType)
	{
		const auto groupDocType = instance().m_groupsMap.at(groupPath).documentType;
		return panda::serialization::canImport(docType, groupDocType);
	}

	panda::PandaObject* GroupsManager::createGroupObject(panda::PandaDocument* document, panda::graphview::GraphView* view, const std::string& groupPath)
	{
		const std::string filePath = instance().m_dirPath + "/" + groupPath + ".grp";
		const auto realPath = panda::helper::system::DataRepository.findFile(filePath);

		panda::XmlDocument doc;
		if (!doc.loadFromFile(realPath))
			throw helper::Exception("Could not open the group file.");

		auto root = doc.root();
		auto registryName = root.attribute("type").toString();

		auto object = panda::ObjectFactory::create(registryName, document);
		if (object)
		{
			object->load(root);
			document->getUndoStack().push(std::make_shared<panda::AddObjectCommand>(document, view->objectsList(), object));
			return object.get();
		}
		else
			return nullptr;
	}

	const GroupsManager::GroupsMap& GroupsManager::groups()
	{
		return instance().m_groupsMap;
	}

	std::string GroupsManager::groupDescription(const std::string& groupPath)
	{
		return instance().m_groupsMap.at(groupPath).description;
	}

	boost::optional<const GroupsManager::GroupInformation&> GroupsManager::groupInformation(const std::string& groupPath)
	{
		const auto& map = instance().m_groupsMap;
		auto it = map.find(groupPath);
		if (it == map.end())
			return {};
		return it->second;
	}

	bool GroupsManager::saveGroup(panda::Group* group, const std::string& groupPath, const std::string& description)
	{
		panda::XmlDocument doc;
		auto root = doc.root();
		root.setName("Group");

		root.setAttribute("description", description);
		root.setAttribute("type", panda::ObjectFactory::registryName(group));
		const auto docType = panda::serialization::getDocumentType(group->parentDocument());
		root.setAttribute("document", panda::serialization::getDocumentName(docType));

		group->save(root);

		const std::string filePath = instance().m_dirPath + "/" + groupPath + ".grp";

		fs::create_directories(fs::path(filePath).parent_path());
		return doc.saveToFile(filePath);
	}

}
