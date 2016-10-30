#include <QtWidgets>

#include <ui/GroupsManager.h>
#include <ui/dialog/QuickCreateDialog.h>

#include <panda/command/AddObjectCommand.h>
#include <panda/document/PandaDocument.h>
#include <panda/graphview/GraphView.h>
#include <panda/object/ObjectFactory.h>

using panda::ObjectFactory;

QuickCreateDialog::QuickCreateDialog(panda::PandaDocument* doc, graphview::GraphView* view, QWidget* parent)
	: QDialog(parent, Qt::Popup)
	, m_document(doc)
	, m_view(view)
{
	QVBoxLayout* vLayout = new QVBoxLayout;

	m_lineEdit = new QLineEdit;
	vLayout->addWidget(m_lineEdit);

	m_listWidget = new QListWidget;
	m_listWidget->setMinimumSize(350, 200);
	vLayout->addWidget(m_listWidget);

	m_descLabel = new QLabel;
	vLayout->addWidget(m_descLabel);

	QPushButton* okButton = new QPushButton(tr("Ok"), this);
	okButton->setDefault(true);
	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
	QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	QHBoxLayout* buttonsLayout = new QHBoxLayout;
	buttonsLayout->addStretch();
	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);
	vLayout->addLayout(buttonsLayout);

	setLayout(vLayout);

	connect(this, SIGNAL(accepted()), this, SLOT(createObject()));
	connect(m_lineEdit, SIGNAL(textEdited(QString)), this, SLOT(searchTextChanged()));
	connect(m_listWidget, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
			this, SLOT(updateDescLabel()));
	connect(m_listWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(accept()));

	m_lineEdit->setFocus(Qt::PopupFocusReason);

	// Get the display text of all objects
	std::vector<std::string> displayNames;
	for(const auto& entry : ObjectFactory::getInstance()->getRegistryMap())
	{
		if (!entry.second.hidden && entry.second.creator->canCreate(doc))
			displayNames.push_back(entry.second.menuDisplay);
	}

	// Remove aliases
	std::sort(displayNames.begin(), displayNames.end());
	auto last = std::unique(displayNames.begin(), displayNames.end());
	displayNames.erase(last, displayNames.end());

	// Convert to a QStringList
	for(const auto& name : displayNames)
		m_menuStringsList << QString::fromStdString(name);

	// Adding groups
	for(const auto& group : GroupsManager::getInstance()->getGroups())
		m_menuStringsList << "Groups/" + group.first;

	m_menuStringsList.sort();
	m_listWidget->addItems(m_menuStringsList);

	updateDescLabel();
}

bool getFactoryEntry(const std::string& menu, ObjectFactory::ClassEntry& entry)
{
	for(const auto& iter : ObjectFactory::getInstance()->getRegistryMap())
	{
		if(menu == iter.second.menuDisplay)
		{
			entry = iter.second;
			return true;
		}
	}

	return false;
}

void QuickCreateDialog::updateDescLabel()
{
	QListWidgetItem* current = m_listWidget->currentItem();
	if(!current)
		current = m_listWidget->item(0);

	if(current)
	{
		QString selectedItemText = current->text();
		if(selectedItemText.startsWith("Groups/"))
		{
			QString groupName = selectedItemText.mid(7);
			QString description = GroupsManager::getInstance()->getGroupDescription(groupName);
			m_descLabel->setText(description);
		}
		else
		{
			ObjectFactory::ClassEntry entry;
			if(getFactoryEntry(selectedItemText.toStdString(), entry))
				m_descLabel->setText(QString::fromStdString(entry.description));
			else
				m_descLabel->setText("");
		}
	}
	else
		m_descLabel->setText("");
}

void QuickCreateDialog::searchTextChanged()
{
	QString text = m_lineEdit->text();
	QStringList searchList = text.split(QRegExp("\\s+"));

// Filter the menu items: keep the ones that contain all search items
	QStringList newMenuList = m_menuStringsList;
	for(const auto& searchItem : searchList)
		newMenuList = newMenuList.filter(searchItem, Qt::CaseInsensitive);

// Reorder them: we want first the items when the searchText is at the start of a word
	typedef QPair<QString, int> StringScore;
	QVector<StringScore> tmpList;
	for(const auto& menuItem : newMenuList)
		tmpList.push_back(qMakePair(menuItem, 0));

	// Increment the score for each searchItem that is at the start of a word
	for(const auto& searchItem : searchList)
	{
		for(auto& tmpItem : tmpList)
		{
			const QString& menuItem = tmpItem.first;
			int from = 0;
			while(true)
			{ // Test all appearances of the searchItem in menuItem
				int pos = menuItem.indexOf(searchItem, from, Qt::CaseInsensitive);
				if(pos == -1)
					break;
				if(!pos || menuItem[pos-1] == ' ' || menuItem[pos-1] == '/')
				{
					++tmpItem.second;
					break;
				}
				from = pos+1;
			}
		}
	}

	// Sort from best score to worst
	std::sort(tmpList.begin(), tmpList.end(), [](const StringScore& lhs, const StringScore& rhs){
		return lhs.second > rhs.second;
	});

	// Copy to the list that will be sent to the widget
	newMenuList.clear();
	for(const auto& tmpItem : tmpList)
		newMenuList.push_back(tmpItem.first);

	QList<QListWidgetItem*> selectedItems = m_listWidget->selectedItems();
	QString selectedItemText;
	if(!selectedItems.empty())
		selectedItemText = selectedItems.front()->text();

	m_listWidget->clear();
	m_listWidget->addItems(newMenuList);

	// Keep the selected item
	if(selectedItemText.size())
	{
		selectedItems = m_listWidget->findItems(selectedItemText, Qt::MatchExactly);
		if(!selectedItems.empty())
			m_listWidget->setCurrentItem(selectedItems.front());
	}

	updateDescLabel();
}

void QuickCreateDialog::createObject()
{
	QList<QListWidgetItem*> selectedItems = m_listWidget->selectedItems();
	QString selectedItemText;
	if(!selectedItems.empty())
		selectedItemText = selectedItems.front()->text();
	else if(m_listWidget->count() >= 1)
		selectedItemText = m_listWidget->item(0)->text();

	if(!selectedItemText.isEmpty())
	{
		if(selectedItemText.startsWith("Groups/"))
		{
			QString groupName = selectedItemText.mid(7);
			GroupsManager::getInstance()->createGroupObject(m_document, m_view, groupName);
		}
		else
		{
			ObjectFactory::ClassEntry entry;
			if(getFactoryEntry(selectedItemText.toStdString(), entry))
			{
				auto object = ObjectFactory::getInstance()->create(entry.className, m_document);
				m_document->getUndoStack().push(std::make_shared<AddObjectCommand>(m_document, m_view->objectsList(), object));
			}
		}
	}

}
