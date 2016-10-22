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
}

namespace graphview {
	class GraphView;
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

	static GroupsManager* getInstance();

	void createGroupsList();
	const GroupsMap& getGroups();
	QString getGroupDescription(const QString& groupName);
	bool canCreate(const QString& groupName, panda::serialization::DocumentType docType);
	bool saveGroup(panda::Group* group);
	panda::PandaObject* createGroupObject(panda::PandaDocument* document, graphview::GraphView* view, QString groupPath);

protected:
	GroupsManager();

	GroupsMap m_groupsMap;
	QString m_groupsDirPath;
};

#endif // GROUPSMANAGER_H
