#include <QtWidgets>

#include <ui/GraphView.h>
#include <ui/GroupsManager.h>
#include <ui/QuickCreateDialog.h>
#include <ui/command/AddObjectCommand.h>

#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>

using panda::ObjectFactory;

QuickCreateDialog::QuickCreateDialog(panda::PandaDocument* doc, GraphView* view)
	: QDialog(view, Qt::Popup)
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

	ObjectFactory* factory = ObjectFactory::getInstance();
	ObjectFactory::RegistryMapIterator iter = factory->getRegistryIterator();
	while(iter.hasNext())
	{
		iter.next();
		if(!iter.value().hidden)
			m_menuStringsList << iter.value().menuDisplay;
	}

	// Adding groups
	GroupsManager::GroupsIterator iter2 = GroupsManager::getInstance()->getGroupsIterator();
	while(iter2.hasNext())
	{
		iter2.next();
		m_menuStringsList << "Groups/" + iter2.key();
	}

	m_menuStringsList.sort();
	m_listWidget->addItems(m_menuStringsList);

	updateDescLabel();
}

bool getFactoryEntry(QString menu, ObjectFactory::ClassEntry& entry)
{
	ObjectFactory* factory = ObjectFactory::getInstance();
	ObjectFactory::RegistryMapIterator iter = factory->getRegistryIterator();
	while(iter.hasNext())
	{
		iter.next();
		if(menu == iter.value().menuDisplay)
		{
			entry = iter.value();
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
			if(getFactoryEntry(selectedItemText, entry))
				m_descLabel->setText(entry.description);
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

	QStringList newMenuList = m_menuStringsList;
	QStringListIterator iter(searchList);
	while(iter.hasNext())
	{
		QString searchItem = iter.next();
		newMenuList = newMenuList.filter(searchItem, Qt::CaseInsensitive);
	}

	QList<QListWidgetItem*> selectedItems = m_listWidget->selectedItems();
	QString selectedItemText;
	if(!selectedItems.empty())
		selectedItemText = selectedItems.front()->text();

	m_listWidget->clear();
	m_listWidget->addItems(newMenuList);

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
			if(getFactoryEntry(selectedItemText, entry))
			{
				auto object = ObjectFactory::getInstance()->create(entry.className, m_document);
				m_document->addCommand(new AddObjectCommand(m_document, m_view, object));
			}
		}
	}

}
