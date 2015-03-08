#include <QtWidgets>

#include <ui/DatasTable.h>
#include <ui/widget/DataWidgetFactory.h>

#include <panda/PandaDocument.h>

DatasTable::DatasTable(panda::PandaDocument* doc, QWidget* parent)
	: QWidget(parent)
	, m_document(doc)
	, m_currentObject(nullptr)
	, m_nextObject(nullptr)
	, m_waitingPopulate(false)
{
	m_nameLabel = new QLabel("Document");
	m_stackedLayout =  new QStackedLayout();

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addWidget(m_nameLabel);
	mainLayout->addLayout(m_stackedLayout);
	setLayout(mainLayout);

	queuePopulate(nullptr);

	connect(doc, SIGNAL(selectedObject(panda::PandaObject*)), this, SLOT(queuePopulate(panda::PandaObject*)));
	connect(doc, SIGNAL(selectedObjectIsDirty(panda::PandaObject*)), this, SLOT(queuePopulate(panda::PandaObject*)));
	connect(doc, SIGNAL(modifiedObject(panda::PandaObject*)), this, SLOT(onModifiedObject(panda::PandaObject*)));
}

void DatasTable::populateTable()
{
	m_waitingPopulate = false;
	if (!m_nextObject)
		m_nextObject = m_document;

	if(m_currentObject == m_nextObject)
	{	// Only update widgets
		// TODO : verify if there are no new datas
		for(DataWidgetPtr dataWidget : m_dataWidgets)
			dataWidget->updateWidgetValue();
		return;
	}

	m_currentObject = m_nextObject;

	QScrollArea* scrollArea = new QScrollArea(this);
	scrollArea->setFrameShape(QFrame::NoFrame);
	scrollArea->setWidgetResizable(true);
	QWidget *layoutWidget = new QWidget(scrollArea);
	QFormLayout *formLayout = new QFormLayout(layoutWidget);
	formLayout->setMargin(0);
	formLayout->setSizeConstraint(QLayout::SetMinimumSize);

	scrollArea->setWidget(layoutWidget);

	if (m_stackedLayout->currentWidget())
		delete m_stackedLayout->currentWidget();
	m_dataWidgets.clear();

	m_nameLabel->setText(m_currentObject->getName());
	m_stackedLayout->addWidget(scrollArea);
	m_stackedLayout->setCurrentWidget(scrollArea);

	auto objectDatas = m_currentObject->getDatas();
	// inputs (or editable)
	for (panda::BaseData* data : objectDatas)
	{
		if (!data->isDisplayed() || data->isReadOnly())
			continue;

		DataWidgetPtr dataWidget = DataWidgetPtr(DataWidgetFactory::getInstance()->create(this, data));

		if (dataWidget)
		{
			m_dataWidgets.push_back(dataWidget);
			bool readOnly = (data->getParent() != nullptr);
			QWidget* widget = dataWidget->createWidgets(readOnly);
			QLabel* label = new QLabel(data->getName(), scrollArea);
			label->setBuddy(widget);
			label->setToolTip(data->getHelp());
			formLayout->addRow(label, widget);
		}
	}

	// outputs (or read only)
	for (panda::BaseData* data : objectDatas)
	{
		if (!data->isDisplayed() || !data->isReadOnly())
			continue;

		DataWidgetPtr dataWidget = DataWidgetPtr(DataWidgetFactory::getInstance()->create(this, data));

		if (dataWidget)
		{
			m_dataWidgets.push_back(dataWidget);

			QWidget* widget = dataWidget->createWidgets(true);
			QLabel* label = new QLabel(data->getName(), scrollArea);
			label->setBuddy(widget);
			label->setToolTip(data->getHelp());
			formLayout->addRow(label, widget);
		}
	}
}

void DatasTable::queuePopulate(panda::PandaObject* object)
{
	if(!m_waitingPopulate)
	{
		m_waitingPopulate = true;
		// The update will happen as soon as all the events in the event queue have been processed
		QTimer::singleShot(0, this, SLOT(populateTable()));
	}

	m_nextObject = object;

	// Bugfix : this is the case where we deselect the object, make sure to refresh later
	//  the bug was that 2 objects (in different documents) were given the same pointer
	if(m_currentObject && !object)
		m_currentObject = nullptr;
}

void DatasTable::onModifiedObject(panda::PandaObject* object)
{
	if(m_currentObject == object)
	{
		// Force the reconstruction of the table
		m_currentObject = nullptr;
		queuePopulate(object);
	}
}

