#include <ui/widget/DataWidgetFactory.h>
#include <ui/widget/EditPolygonDialog.h>
#include <ui/widget/ListDataWidgetDialog.h>
#include <ui/widget/OpenDialogDataWidget.h>
#include <ui/widget/TableDataWidgetDialog.h>

#include <panda/data/Data.h>

#include <QtWidgets>

using panda::types::Path;
using panda::types::Polygon;
using panda::types::DataTypeId;

EditPolygonDialog::EditPolygonDialog(BaseDataWidget* parent, bool readOnly, QString name)
	: QDialog(parent)
{
	QVBoxLayout* mainLayout = new QVBoxLayout();

	QFormLayout* formLayout = new QFormLayout();
	mainLayout->addLayout(formLayout);

	QPushButton* okButton = new QPushButton(tr("Ok"), this);
	okButton->setDefault(true);
	connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
	QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);
	connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
	QHBoxLayout* buttonsLayout = new QHBoxLayout;
	buttonsLayout->addStretch();
	buttonsLayout->addWidget(okButton);
	buttonsLayout->addWidget(cancelButton);
	mainLayout->addLayout(buttonsLayout);
	setLayout(mainLayout);

	m_contourWidget = DataWidgetPtr(DataWidgetFactory::getInstance()
											 ->create(this, &m_polygon.contour,
													  DataTypeId::getIdOf< Path >(),
													  "default", "contour", "")
											 );
	m_holesWidget = DataWidgetPtr(DataWidgetFactory::getInstance()
											 ->create(this, &m_polygon.holes,
													  DataTypeId::getIdOf< std::vector<Path> >(),
													  "default", "holes", "")
											 );

	if (m_contourWidget)
	{
		QWidget* contourWidget = m_contourWidget->createWidgets(readOnly);
		formLayout->addRow("contour", contourWidget);
	}

	if (m_holesWidget)
	{
		QWidget* holesWidget = m_holesWidget->createWidgets(readOnly);
		formLayout->addRow("holes", holesWidget);
	}

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setWindowTitle(name + (readOnly ? tr(" (read-only)") : ""));
}

void EditPolygonDialog::readFromData(const panda::types::Polygon& polygon)
{
	m_polygon = polygon;
	m_contourWidget->updateWidgetValue();
	m_holesWidget->updateWidgetValue();
}

void EditPolygonDialog::writeToData(panda::types::Polygon& polygon)
{
	polygon = m_polygon;
}

//****************************************************************************//

RegisterWidget<OpenDialogDataWidget<panda::types::Polygon, EditPolygonDialog> > DWClass_polygon("default");
RegisterWidget<OpenDialogDataWidget<std::vector<panda::types::Polygon>, ListDataWidgetDialog<std::vector<panda::types::Polygon> > > > DWClass_polygon_list("generic");
