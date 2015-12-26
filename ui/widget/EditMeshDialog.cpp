#include <ui/widget/DataWidgetFactory.h>
#include <ui/widget/EditMeshDialog.h>
#include <ui/widget/ListDataWidgetDialog.h>
#include <ui/widget/OpenDialogDataWidget.h>
#include <ui/widget/TableDataWidgetDialog.h>

#include <panda/Data.h>

#include <QtWidgets>

using panda::types::Mesh;
using panda::types::Point;
using panda::types::DataTypeId;

template<>
class FlatDataTrait<Mesh::Edge>
{
public:
	typedef Mesh::Edge value_type;
	typedef Mesh::PointID item_type;

	static int size() { return 2; }
	static QStringList header()
	{
		QStringList header;
		header << "point id 1" << "point id 2";
		return header;
	}
	static const item_type get(const value_type& d, int i = 0)
	{
		switch(i)
		{
		case 0: return d[0];
		case 1: return d[1];
		}

		return 0.;
	}
	static void set(value_type& d, const item_type& v, int i = 0)
	{
		switch(i)
		{
		case 0: d[0] = v; break;
		case 1: d[1] = v; break;
		}
	}
	static QString toString(const value_type& d)
	{
		return QString("%1 %2").arg(d[0]).arg(d[1]);
	}
};

// Only here to be able to use a DataWidget, not registering into the Factory
template panda::Data< std::vector<Mesh::Edge> >;

typedef OpenDialogDataWidget<std::vector<Mesh::Edge>, TableDataWidgetDialog<std::vector<Mesh::Edge> > > EdgeDataWidget;

//****************************************************************************//

template<>
class FlatDataTrait<Mesh::Triangle>
{
public:
	typedef Mesh::Triangle value_type;
	typedef Mesh::PointID item_type;

	static int size() { return 3; }
	static QStringList header()
	{
		QStringList header;
		header << "point id 1" << "point id 2" << "point id 3";
		return header;
	}
	static const item_type get(const value_type& d, int i = 0)
	{
		switch(i)
		{
		case 0: return d[0];
		case 1: return d[1];
		case 2: return d[2];
		}

		return 0.;
	}
	static void set(value_type& d, const item_type& v, int i = 0)
	{
		switch(i)
		{
		case 0: d[0] = v; break;
		case 1: d[1] = v; break;
		case 2: d[2] = v; break;
		}
	}
	static QString toString(const value_type& d)
	{
		return QString("%1 %2 %3").arg(d[0]).arg(d[1]).arg(d[2]);
	}
};

// Only here to be able to use a DataWidget, not registering into the Factory
template panda::Data< std::vector<Mesh::Triangle> >;

typedef OpenDialogDataWidget<std::vector<Mesh::Triangle>, TableDataWidgetDialog<std::vector<Mesh::Triangle> > > TriangleDataWidget;

//****************************************************************************//

EditMeshDialog::EditMeshDialog(BaseDataWidget* parent, bool readOnly, QString name)
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

	const void* pts = &m_mesh.getPoints();
	m_pointsWidget = DataWidgetPtr(DataWidgetFactory::getInstance()
											 ->create(this, const_cast<void*>(pts),
													  DataTypeId::getIdOf< QVector<Point> >(),
													  "default", "points", "")
											 );

	if (m_pointsWidget)
	{
		QWidget* pointsWidget = m_pointsWidget->createWidgets(readOnly);
		formLayout->addRow("points", pointsWidget);
	}

	const std::vector<Mesh::Edge>* edges = &m_mesh.getEdges();
	m_edgesWidget = DataWidgetPtr(new EdgeDataWidget(this, const_cast<std::vector<Mesh::Edge>*>(edges), "", "edges", ""));
	QWidget* edgesWidget = m_edgesWidget->createWidgets(readOnly);
	formLayout->addRow("edges", edgesWidget);

	const std::vector<Mesh::Triangle>* triangles = &m_mesh.getTriangles();
	m_trianglesWidget = DataWidgetPtr(new TriangleDataWidget(this, const_cast<std::vector<Mesh::Triangle>*>(triangles), "", "triangles", ""));
	QWidget* trianglesWidget = m_trianglesWidget->createWidgets(readOnly);
	formLayout->addRow("triangles", trianglesWidget);

	setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
	setWindowTitle(name + (readOnly ? tr(" (read-only)") : ""));
}

void EditMeshDialog::readFromData(const Mesh& mesh)
{
	m_mesh = mesh;
	m_pointsWidget->updateWidgetValue();
	m_edgesWidget->updateWidgetValue();
	m_trianglesWidget->updateWidgetValue();
}

void EditMeshDialog::writeToData(Mesh& mesh)
{
	mesh = m_mesh;
}

//****************************************************************************//

RegisterWidget<OpenDialogDataWidget<Mesh, EditMeshDialog> > DWClass_mesh("default");
RegisterWidget<OpenDialogDataWidget<QVector<Mesh>, ListDataWidgetDialog<QVector<Mesh> > > > DWClass_meshes_list("generic");
