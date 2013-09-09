#include <ui/SimpleDataWidget.h>
#include <helper/Factory.h>

#include <QtWidgets>

template<>
class data_widget_container< int >
{
public:
	typedef int value_type;
	typedef panda::Data<value_type> data_type;
	QSpinBox* spinBox;

	data_widget_container() : spinBox(nullptr) {}

	QWidget* createWidgets(BaseDataWidget* parent, const data_type*)
	{
		spinBox = new QSpinBox(parent);
		spinBox->setMinimum(INT_MIN);
		spinBox->setMaximum(INT_MAX);
		spinBox->setSingleStep(1);

		QObject::connect(spinBox, SIGNAL(editingFinished()), parent, SLOT(setWidgetDirty()));

		return spinBox;
	}
	void readFromData(const data_type* d)
	{
		int i = d->getValue();
		if(i != spinBox->value())
			spinBox->setValue(i);
	}
	void writeToData(data_type* d)
	{
		d->setValue(spinBox->value());
	}
};

//***************************************************************//

class checkbox_data_widget_trait : public data_widget_container< int >
{
public:
	typedef int value_type;
	typedef panda::Data<value_type> data_type;
	QCheckBox* checkBox;

	checkbox_data_widget_trait() : checkBox(nullptr) {}

	QWidget* createWidgets(BaseDataWidget* parent, const data_type*)
	{
		checkBox = new QCheckBox(parent);
		QObject::connect(checkBox, SIGNAL(stateChanged(int)), parent, SLOT(setWidgetDirty()));
		return checkBox;
	}
	void readFromData(const data_type* d)
	{
		bool b = (d->getValue()!=0);
		if (checkBox->isChecked() != b)
			checkBox->setChecked(b);
	}
	void writeToData(data_type* d)
	{
		d->setValue(checkBox->isChecked() ? 1 : 0);
	}
};

//***************************************************************//

template <>
class data_widget_container< double >
{
public:
	typedef double value_type;
	typedef panda::Data<value_type> data_type;
	QLineEdit* lineEdit;

	data_widget_container() : lineEdit(nullptr) {}

	QWidget* createWidgets(BaseDataWidget* parent, const data_type* /*d*/)
	{
		lineEdit = new QLineEdit(parent);
//		lineEdit->setValidator(new QDoubleValidator(lineEdit));
		QObject::connect(lineEdit, SIGNAL(editingFinished()), parent, SLOT(setWidgetDirty()));
		return lineEdit;
	}
	void readFromData(const data_type* d)
	{
		value_type n = lineEdit->text().toDouble();
		value_type v = d->getValue();
		if (v != n)
			lineEdit->setText(QString::number(v));
	}
	void writeToData(data_type* d)
	{
		bool ok;
		value_type n = lineEdit->text().toDouble(&ok);
		if(ok)
			d->setValue(n);
	}
};

//***************************************************************//

template<>
class data_widget_container< QString >
{
public:
	typedef QString value_type;
	typedef panda::Data<value_type> data_type;
	QLineEdit* lineEdit;

	data_widget_container() : lineEdit(nullptr) {}

	QWidget* createWidgets(BaseDataWidget* parent, const data_type*)
	{
		lineEdit = new QLineEdit(parent);
		QObject::connect(lineEdit, SIGNAL(editingFinished()), parent, SLOT(setWidgetDirty()) );
		return lineEdit;
	}
	void readFromData(const data_type* d)
	{
		value_type v = d->getValue();
		if (lineEdit->text() != v)
			lineEdit->setText(v);
	}
	void writeToData(data_type* d)
	{
		d->setValue(lineEdit->text());
	}
};

//***************************************************************//

DataWidgetColorChooser::DataWidgetColorChooser(QColor color)
	: theColor(color)
{
	pushButton = new QPushButton("...", this);
	QObject::connect(pushButton, SIGNAL(clicked()), this, SLOT(onChooseColor()) );

	QHBoxLayout* layout = new QHBoxLayout();
	layout->setMargin(0);
	layout->addWidget(pushButton);

	setLayout(layout);
}

void DataWidgetColorChooser::onChooseColor()
{
	QColor tmp = QColorDialog::getColor(theColor, this);
	if(tmp.isValid())
	{
		theColor = tmp;
		emit colorEdited();
	}
}

template<>
class data_widget_container< QColor >
{
public:
	typedef QColor value_type;
	typedef panda::Data<value_type> data_type;
	DataWidgetColorChooser* chooser;

	data_widget_container() : chooser(nullptr) {}

	QWidget* createWidgets(BaseDataWidget* parent, const data_type* d)
	{
		chooser = new DataWidgetColorChooser(d->getValue());
		QObject::connect(chooser, SIGNAL(colorEdited()), parent, SLOT(setWidgetDirty()) );
		return chooser;
	}
	void readFromData(const data_type* d)
	{
		chooser->setColor(d->getValue());
	}
	void writeToData(data_type* d)
	{
		d->setValue(chooser->getColor());
	}
};

//***************************************************************//

Creator<DataWidgetFactory, SimpleDataWidget<int> > DWClass_int("default",true);
Creator<DataWidgetFactory, SimpleDataWidget<int, checkbox_data_widget_trait> > DWClass_checkbox("checkbox",true);
Creator<DataWidgetFactory, SimpleDataWidget<double> > DWClass_double("default",true);
Creator<DataWidgetFactory, SimpleDataWidget<QString> > DWClass_string("default",true);

Creator<DataWidgetFactory, SimpleDataWidget<QColor> > DWClass_color("default",true);
