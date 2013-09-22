#include <QtWidgets>

#include <ui/DatasTable.h>

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

	QScrollArea* scrollArea = new QScrollArea(this);
	scrollArea->setFrameShape(QFrame::NoFrame);
	scrollArea->setWidgetResizable(true);
	QWidget *layoutWidget = new QWidget(scrollArea);
    QFormLayout *formLayout = new QFormLayout(layoutWidget);
	formLayout->setMargin(0);
	formLayout->setSizeConstraint(QLayout::SetMinimumSize);

	scrollArea->setWidget(layoutWidget);

	if (stackedLayout->currentWidget())
		delete stackedLayout->currentWidget();
	dataWidgets.clear();

	nameLabel->setText(object->getName());
	stackedLayout->addWidget(scrollArea);
	stackedLayout->setCurrentWidget(scrollArea);

	// inputs (or editable)
	foreach (panda::BaseData* data, object->getDatas())
	{
		if (!data->isDisplayed() || data->isReadOnly())
			continue;

        BaseDataWidget::CreatorArgument arg = { data, this };
        DataWidgetPtr dataWidget = DataWidgetPtr(BaseDataWidget::CreateDataWidget(arg));
		bool readOnly = (data->getParent() != nullptr);

		if (dataWidget)
		{
			dataWidgets.append(dataWidget);
			QWidget* widget = dataWidget->createWidgets(readOnly);
			formLayout->addRow(data->getName(), widget);
		}
	}

	// outputs (or read only)
	foreach (panda::BaseData* data, object->getDatas())
	{
		if (!data->isDisplayed() || !data->isReadOnly())
			continue;

		BaseDataWidget::CreatorArgument arg = { data, this };
		DataWidgetPtr dataWidget = DataWidgetPtr(BaseDataWidget::CreateDataWidget(arg));

		if (dataWidget)
		{
			dataWidgets.append(dataWidget);

			QWidget* widget = dataWidget->createWidgets(true);
			formLayout->addRow(data->getName(), widget);
		}
	}
}

