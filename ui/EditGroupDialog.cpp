#include <QtWidgets>

#include <ui/EditGroupDialog.h>
#include <panda/command/GroupCommand.h>

#include <panda/Group.h>
#include <panda/PandaDocument.h>

using panda::ObjectFactory;

EditGroupDialog::EditGroupDialog(panda::Group* group, QWidget* parent)
	: QDialog(parent)
	, m_group(group)
	, m_selectedData(nullptr)
	, m_selectedRow(-1)
{
	QVBoxLayout* vLayout = new QVBoxLayout;

	QLabel* groupNameLabel = new QLabel(tr("group name:"), this);
	m_editGroupName = new QLineEdit(group->getGroupName(), this);
	QHBoxLayout* groupNameLayout = new QHBoxLayout;
	groupNameLayout->addWidget(groupNameLabel);
	groupNameLayout->addWidget(m_editGroupName);
	vLayout->addLayout(groupNameLayout);

	auto groupDatas = group->getGroupDatas();

	m_tableWidget = new QTableWidget(this);
	m_tableWidget->setColumnCount(4);
	m_tableWidget->setRowCount(groupDatas.size());
	m_tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_tableWidget->verticalHeader()->setEnabled(false);
	m_tableWidget->verticalHeader()->hide();

	QStringList headerLabels;
	headerLabels << "name" << "input" << "output" << "description";
	m_tableWidget->setHorizontalHeaderLabels(headerLabels);
	m_tableWidget->setMinimumWidth(m_tableWidget->horizontalHeader()->length() + 10);
	m_tableWidget->horizontalHeader()->resizeSection(1, 50);
	m_tableWidget->horizontalHeader()->resizeSection(2, 50);
	m_tableWidget->horizontalHeader()->setStretchLastSection(true);
	m_tableWidget->horizontalHeader()->setSectionsClickable(false);

	connect(m_tableWidget, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(itemClicked(QTableWidgetItem*)));

	int rowIndex = 0;
	for(auto data : groupDatas)
	{
		populateRow(rowIndex, const_cast<panda::BaseData*>(data));
		++rowIndex;
	}

	QPushButton* moveUpButton = new QPushButton(tr("Move up"), this);
	connect(moveUpButton, SIGNAL(clicked()), this, SLOT(moveUp()));

	QPushButton* moveDownButton = new QPushButton(tr("Move down"), this);
	connect(moveDownButton, SIGNAL(clicked()), this, SLOT(moveDown()));

	QVBoxLayout* moveButtonsLayout = new QVBoxLayout;
	moveButtonsLayout->addWidget(moveUpButton);
	moveButtonsLayout->addWidget(moveDownButton);

	QHBoxLayout* tableLayout = new QHBoxLayout;
	tableLayout->addWidget(m_tableWidget);
	tableLayout->addLayout(moveButtonsLayout);
	vLayout->addLayout(tableLayout);

	QGridLayout* gridLayout = new QGridLayout;
	QLabel* dataNameLabel = new QLabel(tr("data name:"), this);
	m_editDataName = new QLineEdit(this);
	connect(m_editDataName, SIGNAL(textEdited(QString)), this, SLOT(dataNameEdited(QString)));
	gridLayout->addWidget(dataNameLabel, 0, 0);
	gridLayout->addWidget(m_editDataName, 0, 1);

	QLabel* dataHelpLabel = new QLabel(tr("data description:"), this);
	m_editDataHelp = new QLineEdit(this);
	connect(m_editDataHelp, SIGNAL(textEdited(QString)), this, SLOT(dataHelpEdited(QString)));
	gridLayout->addWidget(dataHelpLabel, 1, 0);
	gridLayout->addWidget(m_editDataHelp, 1, 1);
	vLayout->addLayout(gridLayout);

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

void EditGroupDialog::populateRow(int rowIndex, panda::BaseData* data)
{
	QTableWidgetItem *item0 = new QTableWidgetItem(data->getName());
	item0->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	item0->setData(Qt::UserRole, QVariant::fromValue(static_cast<void*>(data)));
	QTableWidgetItem *item1 = new QTableWidgetItem(data->isInput()?"true":"false");
	item1->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	item1->setData(Qt::UserRole, QVariant::fromValue(static_cast<void*>(data)));
	QTableWidgetItem *item2 = new QTableWidgetItem(data->isOutput()?"true":"false");
	item2->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	item2->setData(Qt::UserRole, QVariant::fromValue(static_cast<void*>(data)));
	QTableWidgetItem *item3 = new QTableWidgetItem(data->getHelp());
	item3->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	item3->setData(Qt::UserRole, QVariant::fromValue(static_cast<void*>(data)));
	m_tableWidget->setItem(rowIndex, 0, item0);
	m_tableWidget->setItem(rowIndex, 1, item1);
	m_tableWidget->setItem(rowIndex, 2, item2);
	m_tableWidget->setItem(rowIndex, 3, item3);
}

void EditGroupDialog::itemClicked(QTableWidgetItem* item)
{
	m_selectedRow = item->row();
	panda::BaseData* newData = (panda::BaseData*)item->data(Qt::UserRole).value<void*>();
	if(newData && newData != m_selectedData)
	{
		m_selectedData = newData;

		m_editDataName->setText(m_tableWidget->item(m_selectedRow, 0)->text());
		m_editDataHelp->setText(m_tableWidget->item(m_selectedRow, 3)->text());
	}
}

void EditGroupDialog::moveUp()
{
	if(!m_selectedData || m_selectedRow <= 0)
		return;

	m_tableWidget->removeRow(m_selectedRow);
	--m_selectedRow;
	m_tableWidget->insertRow(m_selectedRow);
	populateRow(m_selectedRow, m_selectedData);
	m_tableWidget->selectRow(m_selectedRow);
}

void EditGroupDialog::moveDown()
{
	if(!m_selectedData || m_selectedRow >= m_tableWidget->rowCount()-1)
		return;

	m_tableWidget->removeRow(m_selectedRow);
	++m_selectedRow;
	m_tableWidget->insertRow(m_selectedRow);
	populateRow(m_selectedRow, m_selectedData);
	m_tableWidget->selectRow(m_selectedRow);
}

void EditGroupDialog::dataNameEdited(QString text)
{
	if(m_selectedRow < 0)
		return;
	m_tableWidget->item(m_selectedRow, 0)->setText(text);
}

void EditGroupDialog::dataHelpEdited(QString text)
{
	if(m_selectedRow < 0)
		return;
	m_tableWidget->item(m_selectedRow, 3)->setText(text);
}

void EditGroupDialog::updateGroup()
{
	QString groupName = m_group->getGroupName();
	if(m_editGroupName->text().size())
		groupName = m_editGroupName->text();

	EditGroupCommand::DataInfo tempInfo;
	QVector<EditGroupCommand::DataInfo> datasList;
	for(int i=0, nb=m_tableWidget->rowCount(); i<nb; ++i)
	{
		panda::BaseData* data = (panda::BaseData*)m_tableWidget->item(i,0)->data(Qt::UserRole).value<void*>();
		if(data)
		{
			tempInfo.data = data;
			tempInfo.name = m_tableWidget->item(i,0)->text();
			tempInfo.help = m_tableWidget->item(i,3)->text();
			datasList.push_back(tempInfo);
		}
	}

	// Check if we are actually changing something
	bool modified = false;
	if(groupName != m_group->getGroupName())
		modified = true;

	auto groupDatas = m_group->getGroupDatas();
	for(int i=0, nb=m_tableWidget->rowCount(); i<nb; ++i)
	{
		panda::BaseData* data = (panda::BaseData*)m_tableWidget->item(i,0)->data(Qt::UserRole).value<void*>();
		if(data != groupDatas[i]) { modified = true; break; }

		QString name = m_tableWidget->item(i,0)->text();
		if(name != groupDatas[i]->getName()) { modified = true; break; }

		QString help = m_tableWidget->item(i,3)->text();
		if(help != groupDatas[i]->getHelp()) { modified = true; break; }
	}

	if(modified)
		m_group->getParentDocument()->addCommand(new EditGroupCommand(m_group, groupName, datasList));
}
