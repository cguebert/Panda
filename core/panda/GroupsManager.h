#pragma once

#include <panda/document/Serialization.h>

#include <map>

#include <boost/optional.hpp>

namespace panda
{
	class PandaDocument;
	class PandaObject;
	class Group;

	namespace graphview {
		class GraphView;
	}

	class PANDA_CORE_API GroupsManager
	{
	public:
		struct GroupInformation
		{
			std::string description;
			panda::serialization::DocumentType documentType;
		};
		using GroupsMap = std::map<std::string, GroupInformation>;

		static void createGroupsList();
		static const GroupsMap& groups();

		static std::string groupDescription(const std::string& groupPath);
		static boost::optional<const GroupInformation&> groupInformation(const std::string& groupPath);

		static bool canCreate(const std::string& groupPath, panda::serialization::DocumentType docType);
		static panda::PandaObject* createGroupObject(panda::PandaDocument* document, panda::graphview::GraphView* view, const std::string& groupPath);

		static bool saveGroup(panda::Group* group, const std::string& groupPath, const std::string& description);

	protected:
		GroupsManager() = default;
		static GroupsManager& instance();

		GroupsMap m_groupsMap;
		static const std::string m_dirPath;
	};

} // namespace panda
