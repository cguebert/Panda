#include <QtWidgets>

#include <ui/GroupsManager.h>
#include <ui/command/AddObjectCommand.h>

#include <panda/Group.h>
#include <panda/ObjectFactory.h>
#include <panda/PandaDocument.h>

GroupsManager::GroupsManager()
{
	m_groupsDirPath = QCoreApplication::applicationDirPath() + "/groups/";
}

GroupsManager* GroupsManager::getInstance()
{
	static GroupsManager groupManager;
	return &groupManager;
}

void GroupsManager::createGroupsList()
{
	m_groupsMap.clear();
	QStringList nameFilter;
	nameFilter << "*.grp";

	QStack<QString> dirList;
	dirList.push(m_groupsDirPath);
	QDir groupsDir(m_groupsDirPath);

	while(!dirList.isEmpty())
	{
		QDir dir = QDir(dirList.pop());
		QFileInfoList entries = dir.entryInfoList(nameFilter, QDir::Files);
		for(int i=0, nb=entries.size(); i<nb; i++)
		{
			QString desc;
			if(getGroupDescription(entries[i].absoluteFilePath(), desc))
			{
				QString path = groupsDir.relativeFilePath(entries[i].absoluteFilePath());
				int n = path.lastIndexOf(".grp", -1, Qt::CaseInsensitive);
				if(n != -1)
					path = path.left(n);
				m_groupsMap[path] = desc;
			}
		}

		entries = dir.entryInfoList(QStringList(),
			QDir::AllDirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
		for(int i=0, nb=entries.size(); i<nb; i++)
			dirList.push(entries[i].absoluteFilePath());
	}
}

bool GroupsManager::getGroupDescription(const QString &fileName, QString& description)
{
	QFile file(fileName);
	if(!file.open(QIODevice::ReadOnly))
		return false;

	QDomDocument doc;
	if(!doc.setContent(&file))
		return false;

	QDomElement root = doc.documentElement();
	if(!root.hasAttribute("description"))
		return false;
	description = root.attribute("description");

	return true;
}

bool GroupsManager::saveGroup(panda::Group *group)
{
	bool ok;
	QString text = QInputDialog::getText(nullptr, tr("Save group"),
										 tr("Group name:"), QLineEdit::Normal,
										 group->getGroupName(), &ok);
	if (!ok || text.isEmpty())
		return false;

	QString fileName = m_groupsDirPath + text + ".grp";
	QFileInfo fileInfo(fileName);
	QDir dir;
	dir.mkpath(fileInfo.dir().path());
	QFile file(fileName);

	// If already exists
	if(file.exists())
	{
		if(QMessageBox::question(nullptr, tr("Panda"),
							  tr("This group already exists, overwrite?"),
							  QMessageBox::Yes|QMessageBox::No,
							  QMessageBox::Yes)
				!= QMessageBox::Yes)
			return false;
	}

	if (!file.open(QIODevice::WriteOnly))
	{
		QMessageBox::warning(nullptr, tr("Panda"),
							 tr("Cannot write file %1:\n%2.")
							 .arg(file.fileName())
							 .arg(file.errorString()));
		return false;
	}

	QDomDocument doc;
	QDomElement root = doc.createElement("Group");
	doc.appendChild(root);

	QString desc = QInputDialog::getText(nullptr, tr("Save group"),
										 tr("Group description:"), QLineEdit::Normal,
										 "", &ok);

	root.setAttribute("description", desc);
	root.setAttribute("type", panda::ObjectFactory::getRegistryName(group));

	group->save(doc, root);

	file.write(doc.toByteArray(4));
	return true;
}

panda::PandaObject* GroupsManager::createGroupObject(panda::PandaDocument* document, GraphView* view, QString groupPath)
{
	QFile file(m_groupsDirPath + "/" + groupPath + ".grp");
	if(!file.open(QIODevice::ReadOnly))
	{
		QMessageBox::warning(nullptr, tr("Panda"), tr("Could not open the file."));
		return nullptr;
	}

	QDomDocument doc;
	int errLine, errCol;
	if (!doc.setContent(&file, nullptr, &errLine, &errCol))
	{
		QMessageBox::warning(nullptr, tr("Panda"),
							 tr("Cannot parse xml: error in ligne %2, column %3")
							 .arg(errLine)
							 .arg(errCol));
		return false;
	}

	QDomElement root = doc.documentElement();
	QString registryName = root.attribute("type");

	auto object = panda::ObjectFactory::getInstance()->create(registryName, document);
	if(object)
	{
		object->load(root);
		document->addCommand(new AddObjectCommand(document, view, object));
	}
	else
	{
		QMessageBox::warning(nullptr, tr("Panda"),
			tr("Could not create the object %1.\nA plugin must be missing.")
			.arg(registryName));
		return nullptr;
	}

	return object.data();
}

GroupsManager::GroupsIterator GroupsManager::getGroupsIterator()
{
	return GroupsIterator(m_groupsMap);
}

QString GroupsManager::getGroupDescription(const QString& groupName)
{
	return m_groupsMap.value(groupName);
}
