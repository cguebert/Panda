#include <QtGui>

#include <ui/EditGroupDialog.h>

#include <panda/Group.h>

using panda::ObjectFactory;

EditGroupDialog::EditGroupDialog(panda::Group* group, QWidget *parent)
	: QDialog(parent)
	, group(group)
	, selectedData(NULL)
	, selectedRow(-1)
{
    QVBoxLayout* vLayout = new QVBoxLayout;

	QLabel* groupNameLabel = new QLabel(tr("group name:"), this);
	editGroupName = new QLineEdit(group->groupName.getValue(), this);
	QHBoxLayout* groupNameLayout = new QHBoxLayout;
	groupNameLayout->addWidget(groupNameLabel);
	groupNameLayout->addWidget(editGroupName);
	vLayout->addLayout(groupNameLayout);

	tableWidget = new QTableWidget(this);
	tableWidget->setColumnCount(4);
	tableWidget->setRowCount(group->groupDatas.size());
	tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	tableWidget->verticalHeader()->setEnabled(false);
	tableWidget->verticalHeader()->hide();

	QStringList headerLabels;
	headerLabels << "name" << "input" << "output" << "description";
	tableWidget->setHorizontalHeaderLabels(headerLabels);
	tableWidget->setMinimumWidth(tableWidget->horizontalHeader()->length() + 10);
	tableWidget->horizontalHeader()->resizeSection(1, 50);
	tableWidget->horizontalHeader()->resizeSection(2, 50);
	tableWidget->horizontalHeader()->setStretchLastSection(true);
	tableWidget->horizontalHeader()->setClickable(false);

	connect(tableWidget, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(itemClicked(QTableWidgetItem*)));

	populateTable();

	QPushButton* moveUpButton = new QPushButton(tr("Move up"), this);
	connect(moveUpButton, SIGNAL(clicked()), this, SLOT(moveUp()));

	QPushButton* moveDownButton = new QPushButton(tr("Move down"), this);
	connect(moveDownButton, SIGNAL(clicked()), this, SLOT(moveDown()));

	QVBoxLayout* moveButtonsLayout = new QVBoxLayout;
	moveButtonsLayout->addWidget(moveUpButton);
	moveButtonsLayout->addWidget(moveDownButton);

	QHBoxLayout* tableLayout = new QHBoxLayout;
	tableLayout->addWidget(tableWidget);
	tableLayout->addLayout(moveButtonsLayout);
	vLayout->addLayout(tableLayout);

	QLabel* dataNameLabel = new QLabel(tr("data name:"), this);
	editDataName = new QLineEdit(this);
	connect(editDataName, SIGNAL(textEdited(QString)), this, SLOT(dataNameEdited(QString)));
	QHBoxLayout* dataNameLayout = new QHBoxLayout;
	dataNameLayout->addWidget(dataNameLabel);
	dataNameLayout->addWidget(editDataName);
	vLayout->addLayout(dataNameLayout);

	QLabel* dataHelpLabel = new QLabel(tr("data description:"), this);
	editDataHelp = new QLineEdit(this);
	connect(editDataHelp, SIGNAL(textEdited(QString)), this, SLOT(dataHelpEdited(QString)));
	QHBoxLayout* dataHelpLayout = new QHBoxLayout;
	dataHelpLayout->addWidget(dataHelpLabel);
	dataHelpLayout->addWidget(editDataHelp);
	vLayout->addLayout(dataHelpLayout);

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

	connect(this, SIGNAL(accepted()), this, SLOT(updateGroup()));

	setWindowTitle(tr("Edit group"));
}

void EditGroupDialog::populateTable()
{
	int rowIndex = 0;
	foreach(QSharedPointer<panda::BaseData> data, group->groupDatas)
	{
		populateRow(rowIndex, data.data());
		++rowIndex;
	}
}

void EditGroupDialog::populateRow(int rowIndex, panda::BaseData* data)
{
	QTableWidgetItem *item0 = new QTableWidgetItem(data->getName());
	item0->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	item0->setData(Qt::UserRole, QVariant::fromValue((void*)data));
	QTableWidgetItem *item1 = new QTableWidgetItem(data->isInput()?"true":"false");
	item1->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	item1->setData(Qt::UserRole, QVariant::fromValue((void*)data));
	QTableWidgetItem *item2 = new QTableWidgetItem(data->isOutput()?"true":"false");
	item2->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	item2->setData(Qt::UserRole, QVariant::fromValue((void*)data));
	QTableWidgetItem *item3 = new QTableWidgetItem(data->getHelp());
	item3->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	item3->setData(Qt::UserRole, QVariant::fromValue((void*)data));
	tableWidget->setItem(rowIndex, 0, item0);
	tableWidget->setItem(rowIndex, 1, item1);
	tableWidget->setItem(rowIndex, 2, item2);
	tableWidget->setItem(rowIndex, 3, item3);
}

void EditGroupDialog::itemClicked(QTableWidgetItem* item)
{
	selectedRow = item->row();
	panda::BaseData* newData = (panda::BaseData*)item->data(Qt::UserRole).value<void*>();
	if(newData && newData != selectedData)
	{
		selectedData = newData;
		editDataName->setText(selectedData->getName());
		editDataHelp->setText(selectedData->getHelp());
	}
}

void EditGroupDialog::moveUp()
{
	if(!selectedData || selectedRow <= 0)
		return;

	tableWidget->removeRow(selectedRow);
	--selectedRow;
	tableWidget->insertRow(selectedRow);
	populateRow(selectedRow, selectedData);
	tableWidget->selectRow(selectedRow);
}

void EditGroupDialog::moveDown()
{
	if(!selectedData || selectedRow >= tableWidget->rowCount()-1)
		return;

	tableWidget->removeRow(selectedRow);
	++selectedRow;
	tableWidget->insertRow(selectedRow);
	populateRow(selectedRow, selectedData);
	tableWidget->selectRow(selectedRow);
}

void EditGroupDialog::dataNameEdited(QString text)
{
	if(selectedRow < 0)
		return;
	tableWidget->item(selectedRow, 0)->setText(text);
}

void EditGroupDialog::dataHelpEdited(QString text)
{
	if(selectedRow < 0)
		return;
	tableWidget->item(selectedRow, 3)->setText(text);
}

void EditGroupDialog::updateGroup()
{
	if(editGroupName->text().size())
		group->groupName.setValue(editGroupName->text());

	// As I used raw pointers before, I have to do additional work here to get the shared pointers in the right order
	QMap< panda::BaseData*, QSharedPointer<panda::BaseData> > datasPtrMap;
	foreach(QSharedPointer<panda::BaseData> dataPtr, group->groupDatas)
		datasPtrMap.insert(dataPtr.data(), dataPtr);

	QList< QSharedPointer<panda::BaseData> > datasList;
	int nbRows = tableWidget->rowCount();
	for(int i=0; i<nbRows; ++i)
	{
		panda::BaseData* data = (panda::BaseData*)tableWidget->item(i,0)->data(Qt::UserRole).value<void*>();
		if(data)
		{
			data->setName(tableWidget->item(i,0)->text());
			data->setHelp(tableWidget->item(i,3)->text());
			datasList.append(datasPtrMap.value(data));
			group->datas.removeAll(data);
			group->datas.append(data);
		}
	}

	group->groupDatas = datasList;

	group->emitModified();
}