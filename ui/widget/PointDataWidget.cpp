#include <ui/widget/TableDataWidgetDialog.h>
#include <ui/widget/ListDataWidgetDialog.h>
#include <ui/widget/OpenDialogDataWidget.h>
#include <ui/widget/DataWidgetFactory.h>
#include <ui/widget/SimpleDataWidget.h>
#include <panda/types/Path.h>

using panda::types::Point;
using panda::types::Path;

template<>
class FlatDataTrait<Point>
{
public:
	typedef Point value_type;
	typedef qreal item_type;

	static int size() { return 2; }
	static QStringList header()
	{
		QStringList header;
		header << "X" << "Y";
		return header;
	}
	static const item_type get(const value_type& d, int i = 0)
	{
		switch(i)
		{
		case 0: return d.x;
		case 1: return d.y;
		}

		return 0.;
	}
	static void set(value_type& d, const item_type& v, int i = 0)
	{
		switch(i)
		{
		case 0: d.x = v; break;
		case 1: d.y = v; break;
		}
	}
	static QString toString(const value_type& d)
	{
		return QString("%1 %2").arg(d.x).arg(d.y);
	}
};

//***************************************************************//

template <>
class DataWidgetContainer< Point >
{
protected:
	typedef Point value_type;
	QWidget* container;
	QLineEdit *lineEditX, *lineEditY;

public:
	DataWidgetContainer() : lineEditX(nullptr), lineEditY(nullptr) {}

	QWidget* createWidgets(BaseDataWidget* parent, bool readOnly)
	{
		container = new QWidget(parent);
		lineEditX = new QLineEdit("0.0", parent);
		lineEditX->setEnabled(!readOnly);
		QObject::connect(lineEditX, SIGNAL(editingFinished()), parent, SLOT(setWidgetDirty()));

		lineEditY = new QLineEdit("0.0", parent);
		lineEditY->setEnabled(!readOnly);
		QObject::connect(lineEditY, SIGNAL(editingFinished()), parent, SLOT(setWidgetDirty()));

		QHBoxLayout* layout = new QHBoxLayout(container);
		layout->setMargin(0);
		layout->addWidget(lineEditX);
		layout->addWidget(lineEditY);
		container->setLayout(layout);

		return container;
	}
	void readFromData(const value_type& v)
	{
		QString tx = lineEditX->text();
		QString ty = lineEditY->text();
#ifdef PANDA_DOUBLE
		double x = tx.toDouble();
		double y = ty.toDouble();
#else
		double x = tx.toFloat();
		double y = ty.toFloat();
#endif
		if(v.x != x || tx.isEmpty())
			lineEditX->setText(QString::number(v.x));
		if(v.y != y || ty.isEmpty())
			lineEditY->setText(QString::number(v.y));
	}
	void writeToData(value_type& v)
	{
#ifdef PANDA_DOUBLE
		double x = lineEditX->text().toDouble();
		double y = lineEditY->text().toDouble();
#else
		double x = lineEditX->text().toFloat();
		double y = lineEditY->text().toFloat();
#endif
		v = Point(x, y);
	}
};

//RegisterWidget<OpenDialogDataWidget<Point, TableDataWidgetDialog<Point> > > DWClass_point("default");
RegisterWidget<SimpleDataWidget<Point> > DWClass_point("default");
RegisterWidget<OpenDialogDataWidget<QVector<Point>, TableDataWidgetDialog<QVector<Point> > > > DWClass_points_list("default");
RegisterWidget<OpenDialogDataWidget<QVector<Point>, ListDataWidgetDialog<QVector<Point> > > > DWClass_points_list_generic("generic");

//*************************************************************************//

template<>
class VectorDataTrait<Path>
{
public:
	typedef Path vector_type;
	typedef Point row_type;

	enum { is_vector = 1 };
	enum { is_single = 0 };
	static int size(const vector_type& v) { return v.size(); }
	static QStringList header(const vector_type&) { return QStringList(); }
	static const row_type* get(const vector_type& v, int i=0)
	{
		return (i<size(v)) ? &(v[i]) : nullptr;
	}
	static row_type* get(vector_type& v, int i=0)
	{
		return (i<size(v)) ? &(v[i]) : nullptr;
	}
	static void set(vector_type& v, const row_type& r, int i=0)
	{
		if(i < size(v))
			v[i] = r;
	}
	static void resize(vector_type& v, int s)
	{
		v.resize(s);
	}
};

RegisterWidget<OpenDialogDataWidget<Path, TableDataWidgetDialog<Path> > > DWClass_strings_list("default");
RegisterWidget<OpenDialogDataWidget<QVector<Path>, ListDataWidgetDialog<QVector<Path> > > > DWClass_paths_list_generic("generic");
