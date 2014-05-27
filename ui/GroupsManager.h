#ifndef GROUPSMANAGER_H
#define GROUPSMANAGER_H

#include <QObject>
#include <QString>
#include <map>

namespace panda
{
	class PandaDocument;
	class PandaObject;
	class Group;
}

class GraphView;

class GroupsManager : public QObject
{
	Q_OBJECT
public:
	typedef std::map<QString, QString> GroupsMap;

	static GroupsManager* getInstance();

	void createGroupsList();
	const GroupsMap& getGroups();
	QString getGroupDescription(const QString& groupName);
	bool saveGroup(panda::Group* group);
	panda::PandaObject* createGroupObject(panda::PandaDocument* document, GraphView* view, QString groupPath);

protected:
	GroupsManager();
	bool getGroupDescription(const QString &fileName, QString& description);

	GroupsMap m_groupsMap;
	QString m_groupsDirPath;
};

#endif // GROUPSMANAGER_H
