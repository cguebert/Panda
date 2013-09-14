#include <ui/SimpleDataWidget.h>
#include <helper/Factory.h>

#include <QtWidgets>

template<>
class data_widget_container< int >
{
protected:
	typedef int value_type;
	QSpinBox* spinBox;

public:
	data_widget_container() : spinBox(nullptr) {}

	QWidget* createWidgets(BaseDataWidget* parent, bool readOnly)
	{
		spinBox = new QSpinBox(parent);
		spinBox->setMinimum(INT_MIN);
		spinBox->setMaximum(INT_MAX);
		spinBox->setSingleStep(1);
		spinBox->setEnabled(!readOnly);

		QObject::connect(spinBox, SIGNAL(editingFinished()), parent, SLOT(setWidgetDirty()));

		return spinBox;
	}
	void readFromData(const value_type& v)
	{
		if(v != spinBox->value())
			spinBox->setValue(v);
	}
	void writeToData(value_type& v)
	{
		v = spinBox->value();
	}
};

//***************************************************************//

class checkbox_data_widget : public data_widget_container< int >
{
protected:
	typedef int value_type;
	QCheckBox* checkBox;

public:
	checkbox_data_widget() : checkBox(nullptr) {}

	QWidget* createWidgets(BaseDataWidget* parent, bool readOnly)
	{
		checkBox = new QCheckBox(parent);
		checkBox->setEnabled(!readOnly);
		QObject::connect(checkBox, SIGNAL(stateChanged(int)), parent, SLOT(setWidgetDirty()));
		return checkBox;
	}
	void readFromData(const value_type& v)
	{
		bool b = (v!=0);
		if (checkBox->isChecked() != b)
			checkBox->setChecked(b);
	}
	void writeToData(value_type& v)
	{
		v = (checkBox->isChecked() ? 1 : 0);
	}
};

//***************************************************************//

template <>
class data_widget_container< double >
{
protected:
	typedef double value_type;
	QLineEdit* lineEdit;

public:
	data_widget_container() : lineEdit(nullptr) {}

	QWidget* createWidgets(BaseDataWidget* parent, bool readOnly)
	{
		lineEdit = new QLineEdit(parent);
//		lineEdit->setValidator(new QDoubleValidator(lineEdit));
		lineEdit->setEnabled(!readOnly);
		QObject::connect(lineEdit, SIGNAL(editingFinished()), parent, SLOT(setWidgetDirty()));
		return lineEdit;
	}
	void readFromData(const value_type& v)
	{
		value_type n = lineEdit->text().toDouble();
		if (v != n)
			lineEdit->setText(QString::number(v));
	}
	void writeToData(value_type& v)
	{
		bool ok;
		value_type n = lineEdit->text().toDouble(&ok);
		if(ok)
			v = n;
	}
};

//***************************************************************//

template<>
class data_widget_container< QString >
{
protected:
	typedef QString value_type;
	QLineEdit* lineEdit;

public:
	data_widget_container() : lineEdit(nullptr) {}

	QWidget* createWidgets(BaseDataWidget* parent, bool readOnly)
	{
		lineEdit = new QLineEdit(parent);
		lineEdit->setEnabled(!readOnly);
		QObject::connect(lineEdit, SIGNAL(editingFinished()), parent, SLOT(setWidgetDirty()) );
		return lineEdit;
	}
	void readFromData(const value_type& v)
	{
		if (lineEdit->text() != v)
			lineEdit->setText(v);
	}
	void writeToData(value_type& v)
	{
		v = lineEdit->text();
	}
};

//***************************************************************//

Creator<DataWidgetFactory, SimpleDataWidget<int> > DWClass_int("default",true);
Creator<DataWidgetFactory, SimpleDataWidget<int, checkbox_data_widget> > DWClass_checkbox("checkbox",true);
Creator<DataWidgetFactory, SimpleDataWidget<double> > DWClass_double("default",true);
Creator<DataWidgetFactory, SimpleDataWidget<QString> > DWClass_string("default",true);
