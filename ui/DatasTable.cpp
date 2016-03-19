#include <QtWidgets>

#include <ui/DatasTable.h>
#include <ui/GraphView.h>
#include <ui/graph/ObjectsSelection.h>
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

		for (auto data : object->getDatas())
			addInput(*data);
	}

	void disconnect()
	{
		auto inputs = getInputs();
		for (auto input : inputs)
			removeInput(*input);
	}

	void update() override { cleanDirty(); }

	void setDirtyValue(const DataNode* /*caller*/) override
	{
		m_table.updateCurrentObject();
	}

private:
	DatasTable& m_table;
};

//****************************************************************************//

DatasTable::DatasTable(GraphView* view, QWidget* parent)
	: QWidget(parent)
	, m_document(view->getDocument())
	, m_objectWatcher(std::make_unique<ObjectWatcher>(*this))
{
	m_nameLabel = new QLabel("Document");
	m_stackedLayout =  new QStackedLayout();

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addWidget(m_nameLabel);
	mainLayout->addLayout(m_stackedLayout);
	setLayout(mainLayout);

	queuePopulate(nullptr);

	m_observer.get(view->selection().selectedObject).connect<DatasTable, &DatasTable::queuePopulate>(this);
	m_observer.get(m_document->getSignals().dirtyObject).connect<DatasTable, &DatasTable::onDirtyObject>(this);
	m_observer.get(m_document->getSignals().modifiedObject).connect<DatasTable, &DatasTable::onModifiedObject>(this);
	m_observer.get(m_document->getSignals().timeChanged).connect<DatasTable, &DatasTable::updateCurrentObject>(this);
}

DatasTable::~DatasTable() = default;

void DatasTable::populateTable()
{
	m_waitingPopulate = false;
	if (!m_nextObject)
		m_nextObject = m_document;

	if(m_currentObject == m_nextObject)
	{	// Only update widgets
		m_currentObject->updateIfDirty(); // Force the update of the object, as we want the new values
		// TODO : verify if there are no new datas
		for(DataWidgetPtr& dataWidget : m_dataWidgets)
			dataWidget->updateWidgetValue();
		return;
	}

	m_currentObject = m_nextObject;
	m_objectWatcher->connect(m_currentObject);

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

void DatasTable::queuePopulate(panda::PandaObject* object)
{
	if(!m_waitingPopulate)
	{
		m_waitingPopulate = true;
		// The update will happen as soon as all the events in the event queue have been processed
		QTimer::singleShot(0, this, SLOT(populateTable()));
	}

	m_nextObject = object;

	if (m_nextObject != m_currentObject)
		m_objectWatcher->disconnect();

	// Bugfix : this is the case where we deselect the object, make sure to refresh later
	//  the bug was that 2 objects (in different documents) were given the same pointer
	if(m_currentObject && !object)
		m_currentObject = nullptr;
}

void DatasTable::onDirtyObject(panda::PandaObject* object)
{
	if (m_currentObject == object)
	{
		queuePopulate(object);
	}
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
