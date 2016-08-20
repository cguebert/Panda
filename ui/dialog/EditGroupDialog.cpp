#include <QtWidgets>

#include <ui/dialog/EditGroupDialog.h>
#include <panda/command/GroupCommand.h>

#include <panda/object/Group.h>
#include <panda/PandaDocument.h>

using panda::ObjectFactory;

EditGroupDialog::EditGroupDialog(panda::Group* group, QWidget* parent)
	: QDialog(parent)
	, m_group(group)
{
	QVBoxLayout* vLayout = new QVBoxLayout;

	QLabel* groupNameLabel = new QLabel(tr("group name:"), this);
	m_editGroupName = new QLineEdit(QString::fromStdString(group->getGroupName()), this);
	QHBoxLayout* groupNameLayout = new QHBoxLayout;
	groupNameLayout->addWidget(groupNameLabel);
	groupNameLayout->addWidget(m_editGroupName);
	vLayout->addLayout(groupNameLayout);

	auto groupDatas = group->getGroupDatas();
	Datas inputDatas, outputDatas;
	for (const auto& dataSPtr : groupDatas)
	{
		auto data = dataSPtr.get();
		if (data->isInput())
			inputDatas.push_back(data);
		else if (data->isOutput())
			outputDatas.push_back(data);

		auto name = QString::fromStdString(data->getName());
		auto description = QString::fromStdString(data->getHelp());

		m_datasName[data] = name;
		m_datasDescription[data] = description;
	}

	m_inputsListWidget = new QListWidget(this);
	m_outputsListWidget = new QListWidget(this);

	fillList(m_inputsListWidget, inputDatas);
	fillList(m_outputsListWidget, outputDatas);

	QPushButton* moveUpButton = new QPushButton(tr("Move up"), this);
	connect(moveUpButton, &QPushButton::clicked, this, &EditGroupDialog::moveUp);

	QPushButton* moveDownButton = new QPushButton(tr("Move down"), this);
	connect(moveDownButton, &QPushButton::clicked, this, &EditGroupDialog::moveDown);

	QVBoxLayout* moveButtonsLayout = new QVBoxLayout;
	moveButtonsLayout->addStretch();
	moveButtonsLayout->addWidget(moveUpButton);
	moveButtonsLayout->addWidget(moveDownButton);
	moveButtonsLayout->addStretch();

	auto inputsLayout = new QVBoxLayout;
	auto inputsLabel = new QLabel(tr("inputs:"), this);
	inputsLayout->addWidget(inputsLabel);
	inputsLayout->addWidget(m_inputsListWidget);

	auto ouputsLayout = new QVBoxLayout;
	auto outputsLabel = new QLabel(tr("outputs:"), this);
	ouputsLayout->addWidget(outputsLabel);
	ouputsLayout->addWidget(m_outputsListWidget);

	QHBoxLayout* tableLayout = new QHBoxLayout;
	tableLayout->addLayout(inputsLayout);
	tableLayout->addLayout(moveButtonsLayout);
	tableLayout->addLayout(ouputsLayout);
	vLayout->addLayout(tableLayout);

	auto editLayout = new QFormLayout;
	m_dataTypeLabel = new QLabel(this);
	editLayout->addRow("type:", m_dataTypeLabel);

	m_editDataName = new QLineEdit(this);
	connect(m_editDataName, &QLineEdit::textEdited, this, &EditGroupDialog::dataNameEdited);
	editLayout->addRow("name:", m_editDataName);

	m_editDataHelp = new QLineEdit(this);
	connect(m_editDataHelp, &QLineEdit::textEdited, this, &EditGroupDialog::dataHelpEdited);
	editLayout->addRow("description:", m_editDataHelp);
	vLayout->addLayout(editLayout);

	QPushButton* okButton = new QPushButton(tr("Ok"), this);
	okButton->setDefault(true);
	connect(okButton, &QPushButton::clicked, this, &EditGroupDialog::accept);
	QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);
	connect(cancelButton, &QPushButton::clicked, this, &EditGroupDialog::reject);
	QHBoxLayout* buttonsLayout = new QHBoxLayout;
	buttonsLayout->addStretch();
	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);
	vLayout->addLayout(buttonsLayout);

	setLayout(vLayout);

	connect(this, &QDialog::accepted, this, &EditGroupDialog::updateGroup);

	setWindowTitle(tr("Edit group"));
}

void EditGroupDialog::fillList(QListWidget* listWidget, const Datas& datas)
{
	listWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
	listWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	listWidget->setSelectionBehavior(QAbstractItemView::SelectRows);

	for (const auto data : datas)
	{
		auto item = new QListWidgetItem(m_datasName[data]);
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		item->setData(Qt::UserRole, QVariant::fromValue(static_cast<void*>(data)));
		listWidget->addItem(item);
	}

	connect(listWidget, &QListWidget::itemClicked, this, &EditGroupDialog::itemClicked);
}

void EditGroupDialog::itemClicked(QListWidgetItem* item)
{
	m_selection = item;
	panda::BaseData* newData = (panda::BaseData*)item->data(Qt::UserRole).value<void*>();
	if(newData && newData != m_selectedData)
	{
		m_selectedData = newData;

		m_editDataName->setText(m_datasName[m_selectedData]);
		m_editDataHelp->setText(m_datasDescription[m_selectedData]);

		if (m_selectedData->isInput())
			m_dataTypeLabel->setText("input");
		else if (m_selectedData->isOutput())
			m_dataTypeLabel->setText("output");
		else
			m_dataTypeLabel->setText("internal"); // Is it even allowed for group objects?
	}

	auto list = item->listWidget();
	if (list == m_inputsListWidget)
		m_outputsListWidget->clearSelection();
	else
		m_inputsListWidget->clearSelection();
}

void EditGroupDialog::moveUp()
{
	if(!m_selectedData || !m_selection)
		return;

	auto list = m_selection->listWidget();
	auto row = list->row(m_selection);
	if (row <= 0)
		return;

	auto item = list->takeItem(row);
	--row;
	list->insertItem(row, item);
	list->setCurrentRow(row);
}

void EditGroupDialog::moveDown()
{
	if(!m_selectedData || !m_selection)
		return;

	auto list = m_selection->listWidget();
	auto row = list->currentRow();
	if (row >= list->count() - 1)
		return;

	auto item = list->takeItem(row);
	++row;
	list->insertItem(row, item);
	list->setCurrentRow(row);
}

void EditGroupDialog::dataNameEdited(QString text)
{
	if(!m_selectedData || !m_selection)
		return;

	m_selection->setText(text);
	m_datasName[m_selectedData] = text;
}

void EditGroupDialog::dataHelpEdited(QString text)
{
	if(!m_selectedData)
		return;
	m_datasDescription[m_selectedData] = text;
}

void EditGroupDialog::updateGroup()
{
	auto groupName = m_group->getGroupName();
	if(m_editGroupName->text().size())
		groupName = m_editGroupName->text().toStdString();

	// Get the information from the list widgets
	panda::EditGroupCommand::DataInfo tempInfo;
	std::vector<panda::EditGroupCommand::DataInfo> datasList;
	for (int i = 0, nb = m_inputsListWidget->count(); i < nb; ++i)
	{
		auto item = m_inputsListWidget->item(i);
		auto data = static_cast<panda::BaseData*>(item->data(Qt::UserRole).value<void*>());
		tempInfo.data = data;
		tempInfo.name = m_datasName[data].toStdString();
		tempInfo.help = m_datasDescription[data].toStdString();
		datasList.push_back(tempInfo);
	}
	for (int i = 0, nb = m_outputsListWidget->count(); i < nb; ++i)
	{
		auto item = m_outputsListWidget->item(i);
		auto data = static_cast<panda::BaseData*>(item->data(Qt::UserRole).value<void*>());
		tempInfo.data = data;
		tempInfo.name = m_datasName[data].toStdString();
		tempInfo.help = m_datasDescription[data].toStdString();
		datasList.push_back(tempInfo);
	}
	
	// Check if we are actually changing something
	bool modified = false;
	if(groupName != m_group->getGroupName())
		modified = true;

	auto groupDatas = m_group->getGroupDatas();
	if (datasList.size() != groupDatas.size()) // Should not happen?
		modified = true;
	else
	{
		for (int i = 0, nb = groupDatas.size(); i < nb; ++i)
		{
			if(groupDatas[i].get() != datasList[i].data)
			{ modified = true; break; }

			if(groupDatas[i]->getName() != datasList[i].name)
			{ modified = true; break; }

			if(groupDatas[i]->getHelp() != datasList[i].help)
			{ modified = true; break; }
		}
	}
	
	if(modified)
		m_group->parentDocument()->getUndoStack().push(std::make_shared<panda::EditGroupCommand>(m_group, groupName, datasList));
}
