#include <ui/widget/DataWidgetFactory.h>
#include <ui/widget/OpenDialogDataWidget.h>
#include <ui/widget/ListDataWidgetDialog.h>
#include <ui/widget/NumericalDataWidget.h>
#include <ui/widget/TableDataWidgetDialog.h>

#include <QtWidgets>

#include <panda/types/FloatVector.h>
using panda::types::FloatVector;

template <>
class DataWidgetContainer< float >
{
protected:
	typedef float value_type;
	QLineEdit* lineEdit;

public:
	DataWidgetContainer() : lineEdit(nullptr) {}

	QWidget* createWidgets(BaseDataWidget* parent, bool readOnly)
	{
		lineEdit = new QLineEdit(parent);
		lineEdit->setEnabled(!readOnly);
		QObject::connect(lineEdit, SIGNAL(editingFinished()), parent, SLOT(setWidgetDirty()));
		return lineEdit;
	}
	void readFromData(const value_type& v)
	{
		QString t = lineEdit->text();
		value_type n = t.toFloat();
		if (v != n || t.isEmpty())
			lineEdit->setText(QString::number(v));
	}
	void writeToData(value_type& v)
	{
		bool ok;
		value_type n = lineEdit->text().toFloat(&ok);
		if(ok)
			v = n;
	}
};

//****************************************************************************//

template<>
static QString FlatDataTrait<float>::toString(const value_type& d)
{ return QString::number(d); }

template<>
class VectorDataTrait<FloatVector>
{
public:
	typedef FloatVector vector_type;
	typedef float row_type;

	enum { is_vector = 1 };
	enum { is_single = 0 };
	static int size(const vector_type& v) { return v.values.size(); }
	static QStringList header(const vector_type&) { return QStringList(); }
	static const row_type* get(const vector_type& v, int i = 0)
	{
		return (i < size(v)) ? &(v.values[i]) : nullptr;
	}
	static row_type* get(vector_type& v, int i = 0)
	{
		return (i < size(v)) ? &(v.values[i]) : nullptr;
	}
	static void set(vector_type& v, const row_type& r, int i = 0)
	{
		if (i < size(v))
			v.values[i] = r;
	}
	static void resize(vector_type& v, int s)
	{
		v.values.resize(s);
	}
};

//****************************************************************************//

RegisterWidget<SimpleDataWidget<float> > DWClass_double("default");
RegisterWidget<SimpleDataWidget<float, SliderDataWidget<float> > > DWClass_slider_double("slider");
RegisterWidget<OpenDialogDataWidget<std::vector<float>, ListDataWidgetDialog<std::vector<float> > > > DWClass_doubles_list_generic("generic");

RegisterWidget<OpenDialogDataWidget<FloatVector, TableDataWidgetDialog<FloatVector> > > DWClass_strings_list("default");
RegisterWidget<OpenDialogDataWidget<std::vector<FloatVector>, ListDataWidgetDialog<std::vector<FloatVector> > > > DWClass_paths_list_generic("generic");

