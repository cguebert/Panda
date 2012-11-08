#include <QtGui>

#include <ui/LayersTab.h>

#include <panda/PandaDocument.h>

LayersTab::LayersTab(panda::PandaDocument* doc, QWidget *parent)
	: QWidget(parent)
	, document(doc)
{
	QLabel* compositionLabel = new QLabel(tr("Mode"), this);
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

	QLabel* opacityLabel = new QLabel(tr("Opacity"), this);
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

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addLayout(compositionLayout);
	mainLayout->addLayout(opacityLayout);
	mainLayout->addWidget(tableWidget);
	setLayout(mainLayout);

	updateTable();
}

void LayersTab::updateTable()
{

}
