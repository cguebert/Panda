#ifndef GROUPSMANAGER_H
#define GROUPSMANAGER_H

#include <QObject>
#include <QString>
#include <QMap>

namespace panda
{
	class PandaDocument;
	class PandaObject;
	class Group;
}

class GroupsManager : public QObject
{
	Q_OBJECT
public:
	typedef QMapIterator<QString, QString> GroupsIterator;

	static GroupsManager* getInstance();

	void createGroupsList();
	GroupsIterator getGroupsIterator();
	QString getGroupDescription(const QString& groupName);
	bool saveGroup(panda::Group* group);
	panda::PandaObject* createGroupObject(panda::PandaDocument* document, QString groupPath);

protected:
	GroupsManager();
	bool getGroupDescription(const QString &fileName, QString& description);

	QMap<QString, QString> m_groupsMap;
	QString m_groupsDirPath;
};

#endif // GROUPSMANAGER_H
