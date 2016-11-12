#ifndef GROUPSMANAGER_H
#define GROUPSMANAGER_H

#include <QObject>
#include <QString>
#include <map>

#include <panda/document/Serialization.h>

namespace panda
{
	class PandaDocument;
	class PandaObject;
	class Group;

	namespace graphview {
		class GraphView;
	}
}

class GroupsManager : public QObject
{
	Q_OBJECT
public:
	struct GroupInformation
	{
		QString description;
		panda::serialization::DocumentType documentType;
	};
	using GroupsMap = std::map<QString, GroupInformation>;

	static void createGroupsList();
	static const GroupsMap& groups();
	static QString groupDescription(const QString& groupName);
	static bool canCreate(const QString& groupName, panda::serialization::DocumentType docType);
	static bool saveGroup(panda::Group* group);
	static panda::PandaObject* createGroupObject(panda::PandaDocument* document, panda::graphview::GraphView* view, QString groupPath);

protected:
	GroupsManager();
	static GroupsManager& instance();

	GroupsMap m_groupsMap;
	QString m_groupsDirPath;
};

#endif // GROUPSMANAGER_H
