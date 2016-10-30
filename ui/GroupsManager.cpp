#include <QtWidgets>

#include <ui/GroupsManager.h>
#include <ui/graphview/GraphView.h>

#include <panda/command/AddObjectCommand.h>
#include <panda/document/PandaDocument.h>
#include <panda/document/Serialization.h>
#include <panda/object/Group.h>
#include <panda/object/ObjectFactory.h>
#include <panda/XmlDocument.h>

namespace
{

	std::pair<bool, GroupsManager::GroupInformation> getGroupInformation(const QString &fileName)
	{
		panda::XmlDocument doc;
		if (!doc.loadFromFile(fileName.toStdString()))
			return { false, {} };

		auto root = doc.root();
		auto descAtt = root.attribute("description");
		if(!descAtt)
			return { false, {} };

		const auto description = QString::fromStdString(descAtt.toString());

		auto docTypeAtt = root.attribute("document");
		auto docType = panda::serialization::DocumentType::Base;
		if (docTypeAtt)
			docType = panda::serialization::getDocumentType(docTypeAtt.toString());

		return { true, {description, docType} };
	}

}

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
			const auto infoPair = getGroupInformation(entries[i].absoluteFilePath());
			if(infoPair.first)
			{
				QString path = groupsDir.relativeFilePath(entries[i].absoluteFilePath());
				int n = path.lastIndexOf(".grp", -1, Qt::CaseInsensitive);
				if(n != -1)
					path = path.left(n);
				m_groupsMap[path] = infoPair.second;
			}
		}

		entries = dir.entryInfoList(QStringList(),
			QDir::AllDirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
		for(int i=0, nb=entries.size(); i<nb; i++)
			dirList.push(entries[i].absoluteFilePath());
	}
}

bool GroupsManager::saveGroup(panda::Group *group)
{
	bool ok;
	QString text = QInputDialog::getText(nullptr, tr("Save group"),
										 tr("Group name:"), QLineEdit::Normal,
										 QString::fromStdString(group->getGroupName()), &ok);
	if (!ok || text.isEmpty())
		return false;

	QString fileName = m_groupsDirPath + text + ".grp";
	QFileInfo fileInfo(fileName);
	QDir dir;
	dir.mkpath(fileInfo.dir().path());
	QFile file(fileName);
	QString description;

	// If already exists
	if(file.exists())
	{
		if(QMessageBox::question(nullptr, tr("Panda"),
							  tr("This group already exists, overwrite?"),
							  QMessageBox::Yes|QMessageBox::No,
							  QMessageBox::Yes)
				!= QMessageBox::Yes)
			return false;
		description = getGroupInformation(fileName).second.description;
	}

	if (!file.open(QIODevice::WriteOnly))
	{
		QMessageBox::warning(nullptr, tr("Panda"),
							 tr("Cannot write file %1:\n%2.")
							 .arg(file.fileName())
							 .arg(file.errorString()));
		return false;
	}
	file.close();

	panda::XmlDocument doc;
	auto root = doc.root();
	root.setName("Group");

	description = QInputDialog::getText(nullptr, tr("Save group"),
										tr("Group description:"), QLineEdit::Normal,
										description, &ok);

	root.setAttribute("description", description.toStdString());
	root.setAttribute("type", panda::ObjectFactory::getRegistryName(group));
	const auto docType = panda::serialization::getDocumentType(group->parentDocument());
	root.setAttribute("document", panda::serialization::getDocumentName(docType));

	group->save(root);

	doc.saveToFile(fileName.toStdString());
	return true;
}

panda::PandaObject* GroupsManager::createGroupObject(panda::PandaDocument* document, graphview::GraphView* view, QString groupPath)
{
	QString fileName = m_groupsDirPath + "/" + groupPath + ".grp";
	QFile file(fileName);
	if(!file.open(QIODevice::ReadOnly))
	{
		QMessageBox::warning(nullptr, tr("Panda"), tr("Could not open the file."));
		return nullptr;
	}

	panda::XmlDocument doc;
	if (!doc.loadFromFile(fileName.toStdString()))
	{
		QMessageBox::warning(nullptr, tr("Panda"), tr("Xml error"));
		return nullptr;
	}

	auto root = doc.root();
	auto registryName = root.attribute("type").toString();

	auto object = panda::ObjectFactory::getInstance()->create(registryName, document);
	if(object)
	{
		object->load(root);
		document->getUndoStack().push(std::make_shared<AddObjectCommand>(document, view->objectsList(), object));
		return object.get();
	}
	else
		return nullptr;
}

const GroupsManager::GroupsMap& GroupsManager::getGroups()
{
	return m_groupsMap;
}

QString GroupsManager::getGroupDescription(const QString& groupName)
{
	return m_groupsMap.at(groupName).description;
}

bool GroupsManager::canCreate(const QString& groupName, panda::serialization::DocumentType docType)
{
	const auto groupDocType = m_groupsMap.at(groupName).documentType;
	return panda::serialization::canImport(docType, groupDocType);
}
