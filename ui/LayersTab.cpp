#include <QtWidgets>

#include <ui/LayersTab.h>
#include <ui/command/SetDataValueCommand.h>
#include <panda/command/MoveLayerCommand.h>

#include <panda/PandaDocument.h>
#include <panda/Layer.h>

LayersTab::LayersTab(panda::PandaDocument* document, QWidget* parent)
	: QWidget(parent)
	, m_document(document)
	, m_selectedLayer(nullptr)
{
	QLabel* nameLabel = new QLabel(tr("Name:"), this);
	m_nameEdit = new QLineEdit(this);
	QHBoxLayout* nameLayout = new QHBoxLayout;
	nameLayout->addWidget(nameLabel);
	nameLayout->addWidget(m_nameEdit);

	QLabel* compositionLabel = new QLabel(tr("Mode:"), this);
	m_compositionBox = new QComboBox(this);
	QStringList compositionModes;
	compositionModes	<< "SourceOver"
						<< "DestinationOver"
						<< "Clear"
						<< "Source"
						<< "Destination"
						<< "SourceIn"
						<< "DestinationIn"
						<< "SourceOut"
						<< "DestinationOut"
						<< "SourceAtop"
						<< "DestinationAtop"
						<< "Xor"
						<< "Plus"
						<< "Multiply"
						<< "Screen"
						<< "Overlay"
						<< "Darken"
						<< "Lighten"
						<< "ColorDodge"
						<< "ColorBurn"
						<< "HardLight"
						<< "SoftLight"
						<< "Difference"
						<< "Exclusion";
	m_compositionBox->addItems(compositionModes);
	QHBoxLayout* compositionLayout = new QHBoxLayout;
	compositionLayout->addWidget(compositionLabel);
	compositionLayout->addWidget(m_compositionBox);

	QLabel* opacityLabel = new QLabel(tr("Opacity:"), this);
	m_opacitySlider = new QSlider(this);
	m_opacitySlider->setOrientation(Qt::Horizontal);
	m_opacitySlider->setTickPosition(QSlider::NoTicks);
	m_opacitySlider->setMinimum(0);
	m_opacitySlider->setMaximum(100);
	QHBoxLayout* opacityLayout = new QHBoxLayout;
	opacityLayout->addWidget(opacityLabel);
	opacityLayout->addWidget(m_opacitySlider);

	m_tableWidget = new QTableWidget(this);
	m_tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	m_tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	m_tableWidget->setColumnCount(1);
	m_tableWidget->verticalHeader()->setEnabled(false);
	m_tableWidget->verticalHeader()->hide();
	m_tableWidget->horizontalHeader()->setEnabled(false);
	m_tableWidget->horizontalHeader()->hide();
	m_tableWidget->horizontalHeader()->setStretchLastSection(true);

	m_moveUpButton = new QPushButton(tr("Move up"), this);
	m_moveDownButton = new QPushButton(tr("Move down"), this);
	QHBoxLayout* moveButtonsLayout = new QHBoxLayout;
	moveButtonsLayout->addWidget(m_moveUpButton);
	moveButtonsLayout->addWidget(m_moveDownButton);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(nameLayout);
	mainLayout->addLayout(compositionLayout);
	mainLayout->addLayout(opacityLayout);
	mainLayout->addWidget(m_tableWidget);
	mainLayout->addLayout(moveButtonsLayout);
	setLayout(mainLayout);

	connect(m_document, SIGNAL(addedObject(panda::PandaObject*)), this, SLOT(addedObject(panda::PandaObject*)));
	connect(m_document, SIGNAL(removedObject(panda::PandaObject*)), this, SLOT(removedObject(panda::PandaObject*)));
	connect(m_document, SIGNAL(dirtyObject(panda::PandaObject*)), this, SLOT(dirtyObject(panda::PandaObject*)));
	connect(m_document, SIGNAL(reorderedObjects()), this, SLOT(reorderObjects()));

	connect(m_nameEdit, SIGNAL(editingFinished()), this, SLOT(nameChanged()));
	connect(m_tableWidget, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(itemClicked(QTableWidgetItem*)));
	connect(m_compositionBox, SIGNAL(currentIndexChanged(int)), this, SLOT(compositionModeChanged(int)));
	connect(m_opacitySlider, SIGNAL(valueChanged(int)), this, SLOT(opacityChanged(int)));
	connect(m_moveUpButton, SIGNAL(clicked()), this, SLOT(moveLayerUp()));
	connect(m_moveDownButton, SIGNAL(clicked()), this, SLOT(moveLayerDown()));

	m_nameEdit->setEnabled(false);
	m_compositionBox->setEnabled(false);
	m_opacitySlider->setEnabled(false);
	m_moveUpButton->setEnabled(false);
	m_moveDownButton->setEnabled(false);
}

void LayersTab::updateTable()
{
	m_tableWidget->clear();
	int nbRows = m_layers.size();
	m_tableWidget->setRowCount(nbRows);
	int rowIndex = nbRows-1;
	for(panda::BaseLayer* layer : m_layers)
	{
		QTableWidgetItem *item = new QTableWidgetItem(layer->getLayerName());
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		item->setData(Qt::UserRole, QVariant::fromValue(static_cast<void*>(layer)));
		m_tableWidget->setItem(rowIndex, 0, item);
		if(m_selectedLayer == layer)
			m_tableWidget->selectRow(rowIndex);
		--rowIndex;
	}
}

void LayersTab::addedObject(panda::PandaObject* object)
{
	panda::BaseLayer* layer = dynamic_cast<panda::BaseLayer*>(object);
	if(layer)
	{
		if(m_layers.empty())
		{
			m_selectedLayer = layer;
			m_nameEdit->setEnabled(true);
			m_nameEdit->setText(m_selectedLayer->getLayerName());
			m_compositionBox->setEnabled(true);
			m_compositionBox->setCurrentIndex(m_selectedLayer->getCompositionMode());
			m_opacitySlider->setEnabled(true);
			m_opacitySlider->setValue(m_selectedLayer->getOpacity() * 100);
			m_moveUpButton->setEnabled(false);
			m_moveDownButton->setEnabled(false);
		}
		m_layers.push_back(layer);
		updateTable();
	}
}

void LayersTab::removedObject(panda::PandaObject* object)
{
	panda::BaseLayer* layer = dynamic_cast<panda::BaseLayer*>(object);
	if(layer)
	{
		if(m_selectedLayer == layer)
		{
			m_selectedLayer = nullptr;
			m_nameEdit->setText("");
			m_nameEdit->setEnabled(false);
			m_compositionBox->setCurrentIndex(0);
			m_compositionBox->setEnabled(false);
			m_opacitySlider->setValue(100);
			m_opacitySlider->setEnabled(false);
			m_moveUpButton->setEnabled(false);
			m_moveDownButton->setEnabled(false);
		}

		m_layers.removeAll(layer);
		updateTable();
	}
}

void LayersTab::dirtyObject(panda::PandaObject* object)
{
	if(m_selectedLayer && m_selectedLayer == dynamic_cast<panda::BaseLayer*>(object))
	{
		m_nameEdit->setText(m_selectedLayer->getLayerName());
		m_compositionBox->setCurrentIndex(m_selectedLayer->getCompositionMode());
		m_opacitySlider->setValue(m_selectedLayer->getOpacity() * 100);
	}
	updateTable();
}

void LayersTab::itemClicked(QTableWidgetItem* item)
{
	m_selectedLayer = (panda::BaseLayer*)item->data(Qt::UserRole).value<void*>();
	if(m_selectedLayer)
	{
		m_nameEdit->setEnabled(true);
		m_nameEdit->setText(m_selectedLayer->getLayerName());
		m_compositionBox->setEnabled(true);
		m_compositionBox->setCurrentIndex(m_selectedLayer->getCompositionMode());
		m_opacitySlider->setEnabled(true);
		m_opacitySlider->setValue(m_selectedLayer->getOpacity() * 100);
		m_moveUpButton->setEnabled(item->row() > 0);
		m_moveDownButton->setEnabled(item->row() < m_layers.size() - 1);
	}
	else
	{
		m_nameEdit->setEnabled(false);
		m_compositionBox->setEnabled(false);
		m_opacitySlider->setEnabled(false);
		m_moveUpButton->setEnabled(false);
		m_moveDownButton->setEnabled(false);
	}
}

void LayersTab::compositionModeChanged(int mode)
{
	if(m_selectedLayer)
	{
		auto data = &m_selectedLayer->getCompositionModeData();
		auto oldValue = data->getValue();
		auto owner = dynamic_cast<panda::PandaObject*>(m_selectedLayer);
		if (oldValue != mode)
			m_document->addCommand(new SetDataValueCommand<int>(data, oldValue, mode, owner));
	}
}

void LayersTab::opacityChanged(int opacity)
{
	if(m_selectedLayer)
	{
		auto data = &m_selectedLayer->getOpacityData();
		auto oldValue = data->getValue();
		auto owner = dynamic_cast<panda::PandaObject*>(m_selectedLayer);
		PReal newValue = opacity / 100.0;
		if (oldValue != newValue)
			m_document->addCommand(new SetDataValueCommand<PReal>(data, oldValue, newValue, owner));
	}
}

void LayersTab::moveLayerUp()
{
	if(m_selectedLayer)
	{
		int index = m_layers.indexOf(m_selectedLayer) + 1;
		m_moveUpButton->setEnabled(index < m_layers.size() - 1);
		m_moveDownButton->setEnabled(index > 0);

		auto object = dynamic_cast<panda::PandaObject*>(m_selectedLayer);
		m_document->addCommand(new MoveLayerCommand(m_document, object, index));
	}
}

void LayersTab::moveLayerDown()
{
	if(m_selectedLayer)
	{
		int index = m_layers.indexOf(m_selectedLayer) - 1 ;
		m_moveUpButton->setEnabled(index < m_layers.size() - 1);
		m_moveDownButton->setEnabled(index > 0);

		auto object = dynamic_cast<panda::PandaObject*>(m_selectedLayer);
		m_document->addCommand(new MoveLayerCommand(m_document, object, index));
	}
}

void LayersTab::nameChanged()
{
	QString name = m_nameEdit->text();
	if(m_selectedLayer)
	{
		auto data = &m_selectedLayer->getLayerNameData();
		auto oldValue = data->getValue();
		if(oldValue != name)
		{
			auto owner = dynamic_cast<panda::PandaObject*>(m_selectedLayer);
			m_document->addCommand(new SetDataValueCommand<QString>(data, oldValue, name, owner));
			updateTable();
		}
	}
}

void LayersTab::reorderObjects()
{
	QList<panda::BaseLayer*> newList;
	auto objects = m_document->getObjects();
	for(auto object : objects)
	{
		auto layer = dynamic_cast<panda::BaseLayer*>(object.get());
		if(layer)
			newList.push_back(layer);
	}

	if(newList != m_layers)
	{
		m_layers.swap(newList);
		updateTable();
	}
}
