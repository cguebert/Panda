#include <QtWidgets>

#include <ui/ChooseWidgetDialog.h>
#include <ui/widget/DataWidgetFactory.h>

#include <panda/PandaDocument.h>
#include <panda/helper/algorithm.h>

#include <type_traits>
#include <iostream>

ChooseWidgetDialog::ChooseWidgetDialog(panda::BaseData* data, QWidget* parent)
	: QDialog(parent)
	, m_data(data)
{
	assert(data != nullptr);
	QVBoxLayout* vLayout = new QVBoxLayout;

	QFormLayout* formLayout = new QFormLayout;

	m_types = new QComboBox;
	int fullTypeId = data->getDataTrait()->fullTypeId();
	int valueTypeId = panda::types::DataTypeId::getValueType(fullTypeId);
	auto types = DataWidgetFactory::getInstance()->getWidgetNames(fullTypeId);
	if(valueTypeId != fullTypeId)
	{
		panda::helper::concatenate(types, DataWidgetFactory::getInstance()->getWidgetNames(valueTypeId));
		if (panda::helper::count(types, "default") > 1)
			panda::helper::removeOne(types, "default");
	}

	QStringList items;
	for (const auto& type : types)
		items.push_back(type);
	m_types->addItems(items);
	m_types->setCurrentText(QString::fromStdString(data->getWidget()));
	formLayout->addRow(tr("widget:"), m_types);

	m_format = new QLabel;
	m_format->setText("<i>" + tr("unused") + "</i>");
	formLayout->addRow(tr("format:"), m_format);

	m_parameters = new QTextEdit;
	m_parameters->setPlainText(QString::fromStdString(data->getWidgetData()));
	formLayout->addRow(tr("parameters:"), m_parameters);

	vLayout->addLayout(formLayout);

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

	setWindowTitle(tr("Choose widget"));

	connect(this, SIGNAL(accepted()), this, SLOT(changeData()));
	connect(m_types, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(changedType(const QString&)));

	changedType(QString::fromStdString(data->getWidget()));
}

QSize ChooseWidgetDialog::sizeHint() const
{
	return QSize(300, 80);
}

void ChooseWidgetDialog::changedType(const QString& type)
{
	auto entry = DataWidgetFactory::getInstance()->getEntry(m_data->getDataTrait()->fullTypeId(), type);
	if(entry)
		m_format->setText(entry->creator->getParametersFormat());

	if(m_format->text().isEmpty())
	{
		m_format->setText("<i>" + tr("unused") + "</i>");
		m_parameters->setEnabled(false);
	}
	else
		m_parameters->setEnabled(true);
}

void ChooseWidgetDialog::changeData()
{
	m_data->setWidget(m_types->currentText().toStdString());
	if(m_format->text().isEmpty())
		m_data->setWidgetData("");
	else
		m_data->setWidgetData(m_parameters->toPlainText().toStdString());
	m_data->forceSet();
}
