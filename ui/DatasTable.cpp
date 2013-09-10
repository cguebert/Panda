#include <QtWidgets>

#include <ui/DatasTable.h>
#include <ui/DatasEditDialog.h>

#include <panda/PandaObject.h>

#include <helper/Factory.h>

DatasTable::DatasTable(panda::PandaObject* doc, QWidget *parent)
	: QWidget(parent)
	, document(doc)
	, currentObject(nullptr)
{
	nameLabel = new QLabel("Document");
	stackedLayout =  new QStackedLayout();

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addWidget(nameLabel);
	mainLayout->addLayout(stackedLayout);
	setLayout(mainLayout);

	populateTable((panda::PandaObject*)doc);

	connect(doc, SIGNAL(selectedObject(panda::PandaObject*)), this, SLOT(populateTable(panda::PandaObject*)));
	connect(doc, SIGNAL(selectedObjectIsDirty(panda::PandaObject*)), this, SLOT(populateTable(panda::PandaObject*)));
}

void DatasTable::populateTable(panda::PandaObject* object)
{
	if (!object)
		object = document;

	if(currentObject == object)
	{	// Only update widgets
		// TODO : verify if there are no new datas
		foreach(DataWidgetPtr dataWidget, dataWidgets)
			dataWidget->updateWidgetValue();
		return;
	}

	currentObject = object;

	QWidget *layoutWidget = new QWidget(this);
    QFormLayout *formLayout = new QFormLayout(layoutWidget);

	if (stackedLayout->currentWidget())
		delete stackedLayout->currentWidget();
	dataWidgets.clear();

	nameLabel->setText(object->getName());
	stackedLayout->addWidget(layoutWidget);
	stackedLayout->setCurrentWidget(layoutWidget);

	// inputs (or editable)
	foreach (panda::BaseData* data, object->getDatas())
	{
		if (!data->isDisplayed() || data->isReadOnly())
			continue;

        BaseDataWidget::CreatorArgument arg = { data, this };
        DataWidgetPtr dataWidget = DataWidgetPtr(BaseDataWidget::CreateDataWidget(arg));

		if (dataWidget)
		{
			dataWidgets.append(dataWidget);

			QWidget* widget = dataWidget->createWidgets();
		//	widget->setEnabled(data->getParent() == nullptr);
			formLayout->addRow(data->getName(), widget);
		}
		else
		{
			QString text = data->toString();
			text.truncate(150);
			QLineEdit* lineEdit = new QLineEdit();
			lineEdit->setText(text);
			lineEdit->setEnabled(data->getParent() == nullptr);
			formLayout->addRow(data->getName(), lineEdit);
		}
	}

	// outputs (or read only)
	foreach (panda::BaseData* data, object->getDatas())
	{
		if (!data->isDisplayed() || !data->isReadOnly())
			continue;

		QLineEdit* lineEdit = new QLineEdit(data->toString(), layoutWidget);
		lineEdit->setEnabled(false);
		formLayout->addRow(data->getName(), lineEdit);
	}
}

