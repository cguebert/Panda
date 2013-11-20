#include <QtWidgets>

#include <ui/LayersTab.h>

#include <panda/PandaDocument.h>
#include <panda/Layer.h>

LayersTab::LayersTab(panda::PandaDocument* doc, QWidget *parent)
	: QWidget(parent)
	, document(doc)
	, selectedLayer(nullptr)
{
	QLabel* nameLabel = new QLabel(tr("Name:"), this);
	nameEdit = new QLineEdit(this);
	QHBoxLayout* nameLayout = new QHBoxLayout;
	nameLayout->addWidget(nameLabel);
	nameLayout->addWidget(nameEdit);

	QLabel* compositionLabel = new QLabel(tr("Mode:"), this);
	compositionBox = new QComboBox(this);
	QStringList compositionModes;
	compositionModes	<< "SourceOver"
						<< "DestinationOver"
						<< "Clear"
						<< "Source"
						<< "Destination"
						<< "SourceIn"
						<< "DestinationIn"
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
						<< "Exclusion"
						<< "SourceOrDestination"
						<< "SourceAndDestination"
						<< "SourceXorDestination"
						<< "NotSourceAndNotDestination"
						<< "NotSourceOrNotDestination"
						<< "NotSourceXorDestination"
						<< "NoSource"
						<< "NoSourceAndDestination"
						<< "SourceAndNotDestination";
	compositionBox->addItems(compositionModes);
	QHBoxLayout* compositionLayout = new QHBoxLayout;
	compositionLayout->addWidget(compositionLabel);
	compositionLayout->addWidget(compositionBox);

	QLabel* opacityLabel = new QLabel(tr("Opacity:"), this);
	opacitySlider = new QSlider(this);
	opacitySlider->setOrientation(Qt::Horizontal);
	opacitySlider->setTickPosition(QSlider::NoTicks);
	opacitySlider->setMinimum(0);
	opacitySlider->setMaximum(100);
	QHBoxLayout* opacityLayout = new QHBoxLayout;
	opacityLayout->addWidget(opacityLabel);
	opacityLayout->addWidget(opacitySlider);

	tableWidget = new QTableWidget(this);
	tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
	tableWidget->setColumnCount(1);
	tableWidget->verticalHeader()->setEnabled(false);
	tableWidget->verticalHeader()->hide();
	tableWidget->horizontalHeader()->setEnabled(false);
	tableWidget->horizontalHeader()->hide();
	tableWidget->horizontalHeader()->setStretchLastSection(true);

	moveUpButton = new QPushButton(tr("Move up"), this);
	moveDownButton = new QPushButton(tr("Move down"), this);
	QHBoxLayout* moveButtonsLayout = new QHBoxLayout;
	moveButtonsLayout->addWidget(moveUpButton);
	moveButtonsLayout->addWidget(moveDownButton);

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(nameLayout);
	mainLayout->addLayout(compositionLayout);
	mainLayout->addLayout(opacityLayout);
	mainLayout->addWidget(tableWidget);
	mainLayout->addLayout(moveButtonsLayout);
	setLayout(mainLayout);

	connect(doc, SIGNAL(addedObject(panda::PandaObject*)), this, SLOT(addedObject(panda::PandaObject*)));
	connect(doc, SIGNAL(removedObject(panda::PandaObject*)), this, SLOT(removedObject(panda::PandaObject*)));

	connect(nameEdit, SIGNAL(editingFinished()), this, SLOT(nameChanged()));
	connect(tableWidget, SIGNAL(itemClicked(QTableWidgetItem*)), this, SLOT(itemClicked(QTableWidgetItem*)));
	connect(compositionBox, SIGNAL(currentIndexChanged(int)), this, SLOT(compositionModeChanged(int)));
	connect(opacitySlider, SIGNAL(valueChanged(int)), this, SLOT(opacityChanged(int)));
	connect(moveUpButton, SIGNAL(clicked()), this, SLOT(moveLayerUp()));
	connect(moveDownButton, SIGNAL(clicked()), this, SLOT(moveLayerDown()));

	nameEdit->setEnabled(false);
	compositionBox->setEnabled(false);
	opacitySlider->setEnabled(false);
	moveUpButton->setEnabled(false);
	moveDownButton->setEnabled(false);
}

void LayersTab::updateTable()
{
	tableWidget->clear();
	int nbRows = layers.size();
	tableWidget->setRowCount(nbRows);
	int rowIndex = nbRows-1;
	for(panda::BaseLayer* layer : layers)
	{
		QTableWidgetItem *item = new QTableWidgetItem(layer->getLayerName());
		item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		item->setData(Qt::UserRole, QVariant::fromValue(static_cast<void*>(layer)));
		tableWidget->setItem(rowIndex, 0, item);
		if(selectedLayer == layer)
			tableWidget->selectRow(rowIndex);
		--rowIndex;
	}
}

void LayersTab::addedObject(panda::PandaObject* object)
{
	panda::BaseLayer* layer = dynamic_cast<panda::BaseLayer*>(object);
	if(layer)
	{
		connect(object, SIGNAL(dirty(panda::PandaObject*)), this, SLOT(dirtyObject(panda::PandaObject*)));
		if(layers.empty())
		{
			selectedLayer = layer;
			nameEdit->setEnabled(true);
			compositionBox->setEnabled(true);
			compositionBox->setCurrentIndex(selectedLayer->getCompositionMode());
			opacitySlider->setEnabled(true);
			opacitySlider->setValue(selectedLayer->getOpacity() * 100);
			moveUpButton->setEnabled(false);
			moveDownButton->setEnabled(false);
		}
		layers.append(layer);
		updateTable();
	}
}

void LayersTab::removedObject(panda::PandaObject* object)
{
	panda::BaseLayer* layer = dynamic_cast<panda::BaseLayer*>(object);
	if(layer)
	{
		if(selectedLayer == layer)
		{
			selectedLayer = nullptr;
			nameEdit->setEnabled(false);
			compositionBox->setEnabled(false);
			opacitySlider->setEnabled(false);
			moveUpButton->setEnabled(false);
			moveDownButton->setEnabled(false);
		}

		layers.removeAll(layer);
		updateTable();
	}
}

void LayersTab::dirtyObject(panda::PandaObject*)
{
	if(selectedLayer)
	{
		nameEdit->setText(selectedLayer->getLayerName());
		compositionBox->setCurrentIndex(selectedLayer->getCompositionMode());
		opacitySlider->setValue(selectedLayer->getOpacity() * 100);
	}
	updateTable();
}

void LayersTab::itemClicked(QTableWidgetItem* item)
{
	selectedLayer = (panda::BaseLayer*)item->data(Qt::UserRole).value<void*>();
	if(selectedLayer)
	{
		nameEdit->setEnabled(true);
		nameEdit->setText(selectedLayer->getLayerName());
		compositionBox->setEnabled(true);
		compositionBox->setCurrentIndex(selectedLayer->getCompositionMode());
		opacitySlider->setEnabled(true);
		opacitySlider->setValue(selectedLayer->getOpacity() * 100);
		moveUpButton->setEnabled(item->row() > 0);
		moveDownButton->setEnabled(item->row() < layers.size() - 1);
	}
	else
	{
		nameEdit->setEnabled(false);
		compositionBox->setEnabled(false);
		opacitySlider->setEnabled(false);
		moveUpButton->setEnabled(false);
		moveDownButton->setEnabled(false);
	}
}

void LayersTab::compositionModeChanged(int mode)
{
	if(selectedLayer)
		selectedLayer->setCompositionMode(mode);
}

void LayersTab::opacityChanged(int opacity)
{
	if(selectedLayer)
		selectedLayer->setOpacity(opacity / 100.0);
}

void LayersTab::moveLayerUp()
{
	if(selectedLayer)
	{
		int index = layers.indexOf(selectedLayer);
		if(index < layers.size()-1)
		{
			layers.removeAll(selectedLayer);
			++index;
			layers.insert(index, selectedLayer);
			moveUpButton->setEnabled(index < layers.size() - 1);
			moveDownButton->setEnabled(index > 0);
		}
		document->moveLayerUp(dynamic_cast<panda::PandaObject*>(selectedLayer));
		updateTable();
	}
}

void LayersTab::moveLayerDown()
{
	if(selectedLayer)
	{
		int index = layers.indexOf(selectedLayer);
		if(index > 0)
		{
			layers.removeAll(selectedLayer);
			--index;
			layers.insert(index, selectedLayer);
			moveUpButton->setEnabled(index < layers.size() - 1);
			moveDownButton->setEnabled(index > 0);
		}
		document->moveLayerDown(dynamic_cast<panda::PandaObject*>(selectedLayer));
		updateTable();
	}
}

void LayersTab::nameChanged()
{
	QString name = nameEdit->text();
	if(selectedLayer && !name.isEmpty())
	{
		selectedLayer->setLayerName(name);
		updateTable();
	}
}
