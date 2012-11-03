#include <QtGui>

#include <ui/QuickCreateDialog.h>

#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>

using panda::ObjectFactory;

QuickCreateDialog::QuickCreateDialog(panda::PandaDocument* doc, QWidget *parent)
	: QDialog(parent, Qt::Popup)
	, document(doc)
{
    QVBoxLayout* vLayout = new QVBoxLayout;

	lineEdit = new QLineEdit;
	vLayout->addWidget(lineEdit);

	listWidget = new QListWidget;
	listWidget->setMinimumSize(350, 200);
	vLayout->addWidget(listWidget);

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
	connect(lineEdit, SIGNAL(textEdited(QString)), this, SLOT(searchTextChanged()));
	connect(listWidget, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(accept()));

	lineEdit->setFocus(Qt::PopupFocusReason);

	ObjectFactory* factory = ObjectFactory::getInstance();
	ObjectFactory::RegistryMapIterator iter = factory->getRegistryIterator();
	while(iter.hasNext())
	{
		iter.next();
		menuStringsList << iter.value()->menuDisplay;
	}
	listWidget->addItems(menuStringsList);
}

void QuickCreateDialog::searchTextChanged()
{
	QString text = lineEdit->text();
	QStringList searchList = text.split(QRegExp("\\s+"));

	QStringList newMenuList = menuStringsList;
	QStringListIterator iter(searchList);
	while(iter.hasNext())
	{
		QString searchItem = iter.next();
		newMenuList = newMenuList.filter(searchItem, Qt::CaseInsensitive);
	}

	QList<QListWidgetItem*> selectedItems = listWidget->selectedItems();
	QString selectedItemText;
	if(!selectedItems.empty())
		selectedItemText = selectedItems.front()->text();

	listWidget->clear();
	listWidget->addItems(newMenuList);

	if(selectedItemText.size())
	{
		selectedItems = listWidget->findItems(selectedItemText, Qt::MatchExactly);
		if(!selectedItems.empty())
			listWidget->setCurrentItem(selectedItems.front());
	}
}

void QuickCreateDialog::createObject()
{
	QList<QListWidgetItem*> selectedItems = listWidget->selectedItems();
	QString selectedItemText;
	if(!selectedItems.empty())
		selectedItemText = selectedItems.front()->text();
	else if(listWidget->count() >= 1)
		selectedItemText = listWidget->item(0)->text();

	if(selectedItemText.size())
	{
		ObjectFactory* factory = ObjectFactory::getInstance();
		ObjectFactory::RegistryMapIterator iter = factory->getRegistryIterator();
		while(iter.hasNext())
		{
			iter.next();
			if(selectedItemText == iter.value()->menuDisplay)
			{
				document->createObject(iter.value()->className);
				return;
			}
		}
	}

}