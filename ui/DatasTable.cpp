#include <QtWidgets>

#include <ui/DatasTable.h>
#include <ui/widget/DataWidgetFactory.h>

#include <panda/PandaObject.h>

DatasTable::DatasTable(panda::PandaObject* doc, QWidget *parent)
	: QWidget(parent)
	, document(doc)
	, currentObject(nullptr)
	, nextObject(nullptr)
	, waitingPopulate(false)
{
	nameLabel = new QLabel("Document");
	stackedLayout =  new QStackedLayout();

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addWidget(nameLabel);
	mainLayout->addLayout(stackedLayout);
	setLayout(mainLayout);

	queuePopulate((panda::PandaObject*)doc);

	connect(doc, SIGNAL(selectedObject(panda::PandaObject*)), this, SLOT(queuePopulate(panda::PandaObject*)));
	connect(doc, SIGNAL(selectedObjectIsDirty(panda::PandaObject*)), this, SLOT(queuePopulate(panda::PandaObject*)));
	connect(doc, SIGNAL(modifiedObject(panda::PandaObject*)), this, SLOT(onModifiedObject(panda::PandaObject*)));
}

void DatasTable::populateTable()
{
	waitingPopulate = false;
	if (!nextObject)
		nextObject = document;

	if(currentObject == nextObject)
	{	// Only update widgets
		// TODO : verify if there are no new datas
		for(DataWidgetPtr dataWidget : dataWidgets)
			dataWidget->updateWidgetValue();
		return;
	}

	currentObject = nextObject;

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

	nameLabel->setText(currentObject->getName());
	stackedLayout->addWidget(scrollArea);
	stackedLayout->setCurrentWidget(scrollArea);

	// inputs (or editable)
	for (panda::BaseData* data : currentObject->getDatas())
	{
		if (!data->isDisplayed() || data->isReadOnly())
			continue;

		DataWidgetPtr dataWidget = DataWidgetPtr(DataWidgetFactory::getInstance()->create(this, data));

		if (dataWidget)
		{
			dataWidgets.append(dataWidget);
			bool readOnly = (data->getParent() != nullptr);
			QWidget* widget = dataWidget->createWidgets(readOnly);
			formLayout->addRow(data->getName(), widget);
		}
	}

	// outputs (or read only)
	for (panda::BaseData* data : currentObject->getDatas())
	{
		if (!data->isDisplayed() || !data->isReadOnly())
			continue;

		DataWidgetPtr dataWidget = DataWidgetPtr(DataWidgetFactory::getInstance()->create(this, data));

		if (dataWidget)
		{
			dataWidgets.append(dataWidget);

			QWidget* widget = dataWidget->createWidgets(true);
			formLayout->addRow(data->getName(), widget);
		}
	}
}

void DatasTable::queuePopulate(panda::PandaObject* object)
{
	if(!waitingPopulate)
	{
		waitingPopulate = true;
		// The update will happen as soon as all the events in the event queue have been processed
		QTimer::singleShot(0, this, SLOT(populateTable()));
	}

	nextObject = object;

	// Bugfix : this is the case where we deselect the object, make sure to refresh later
	//  the bug was that 2 objects (in different documents) were given the same pointer
	if(currentObject && !object)
		currentObject = nullptr;
}

void DatasTable::onModifiedObject(panda::PandaObject* object)
{
	if(currentObject == object)
	{
		// Force the reconstruction of the table
		currentObject = nullptr;
		queuePopulate(object);
	}
}

