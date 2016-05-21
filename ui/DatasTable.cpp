#include <QtWidgets>

#include <ui/DatasTable.h>
#include <ui/graphview/GraphView.h>
#include <ui/graphview/ObjectsSelection.h>
#include <ui/widget/DataWidgetFactory.h>

#include <panda/PandaDocument.h>
#include <panda/document/DocumentSignals.h>

// A small class to be notified when the outputs of an object change
class ObjectWatcher : public panda::DataNode
{
public:
	ObjectWatcher(DatasTable& table)
		: m_table(table) {}

	void connect(panda::PandaObject* object)
	{
		disconnect();

		m_changingConnections = true;
		for (auto data : object->getDatas())
			addInput(*data);
		m_changingConnections = false;
	}

	void disconnect()
	{
		m_changingConnections = true;
		auto inputs = getInputs();
		for (auto input : inputs)
			removeInput(*input);
		m_changingConnections = false;
	}

	void update() override { cleanDirty(); }

	void setDirtyValue(const DataNode* /*caller*/) override
	{
		if(!m_changingConnections)
			m_table.updateCurrentObject();
	}

private:
	DatasTable& m_table;
	bool m_changingConnections = false;
};

//****************************************************************************//

DatasTable::DatasTable(panda::PandaDocument* document, QWidget* parent)
	: QWidget(parent)
	, m_document(document)
	, m_objectWatcher(std::make_unique<ObjectWatcher>(*this))
{
	m_nameLabel = new QLabel("Document");
	m_stackedLayout =  new QStackedLayout();

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addWidget(m_nameLabel);
	mainLayout->addLayout(m_stackedLayout);
	setLayout(mainLayout);

	queuePopulate(nullptr);

	m_observer.get(m_document->getSignals().modifiedObject).connect<DatasTable, &DatasTable::onModifiedObject>(this);
	m_observer.get(m_document->getSignals().timeChanged).connect<DatasTable, &DatasTable::updateCurrentObject>(this);
}

DatasTable::~DatasTable() = default;

void DatasTable::populateTable()
{
	m_waitingPopulate = false;
	if (!m_nextObject)
		m_nextObject = m_document;

	// If possible, only update widgets
	if(!m_objectIsModified && m_currentObject == m_nextObject)
	{
		m_currentObject->updateIfDirty(); // Force the update of the object, as we want the new values
		// TODO : verify if there are no new datas
		for(DataWidgetPtr& dataWidget : m_dataWidgets)
			dataWidget->updateWidgetValue();
		return;
	}

	m_currentObject = m_nextObject;
	m_objectWatcher->connect(m_currentObject);
	m_objectIsModified = false;

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

	m_nameLabel->setText(QString::fromStdString(m_currentObject->getName()));
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
			QLabel* label = new QLabel(QString::fromStdString(data->getName()), scrollArea);
			label->setBuddy(widget);
			label->setToolTip(QString::fromStdString(data->getHelp()));
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
			QLabel* label = new QLabel(QString::fromStdString(data->getName()), scrollArea);
			label->setBuddy(widget);
			label->setToolTip(QString::fromStdString(data->getHelp()));
			formLayout->addRow(label, widget);
		}
	}
}

void DatasTable::updateCurrentObject()
{
	queuePopulate(m_currentObject);
}

void DatasTable::setSelectedObject(panda::PandaObject* object)
{
	queuePopulate(object);
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
		m_objectIsModified = true;
		queuePopulate(object);
	}
}
