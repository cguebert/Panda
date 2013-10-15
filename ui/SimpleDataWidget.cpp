#include <ui/SimpleDataWidget.h>
#include <panda/helper/Random.h>

#include <QtWidgets>

template<>
class DataWidgetContainer< int >
{
protected:
	typedef int value_type;
	QSpinBox* spinBox;

public:
	DataWidgetContainer() : spinBox(nullptr) {}

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

class CheckboxDataWidget : public DataWidgetContainer< int >
{
protected:
	typedef int value_type;
	QCheckBox* checkBox;

public:
	CheckboxDataWidget() : checkBox(nullptr) {}

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

class SeedDataWidget : public DataWidgetContainer< int >, public BaseOpenDialogObject
{
protected:
	typedef int value_type;
	QSpinBox* spinBox;
	BaseDataWidget* m_parent;

public:
	SeedDataWidget() : spinBox(nullptr) {}

	QWidget* createWidgets(BaseDataWidget* parent, bool readOnly)
	{
		m_parent = parent;
		QWidget* container = new QWidget(parent);

		spinBox = new QSpinBox();
		spinBox->setMinimum(INT_MIN);
		spinBox->setMaximum(INT_MAX);
		spinBox->setSingleStep(1);
		spinBox->setEnabled(!readOnly);

		QPushButton* button = new QPushButton("Random");
		QObject::connect(button, SIGNAL(clicked()), this, SLOT(onShowDialog()));

		QHBoxLayout* layout = new QHBoxLayout;
		layout->setMargin(0);
		layout->addWidget(spinBox, 1);
		layout->addWidget(button);
		container->setLayout(layout);

		QObject::connect(spinBox, SIGNAL(editingFinished()), parent, SLOT(setWidgetDirty()));

		return container;
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
	void onShowDialog()
	{
		spinBox->setValue(panda::helper::RandomGenerator::getRandomSeed(10000));
		m_parent->setWidgetDirty();
	}
};

//***************************************************************//

template <>
class DataWidgetContainer< double >
{
protected:
	typedef double value_type;
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
		value_type n = t.toDouble();
		if (v != n || t.isEmpty())
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
class DataWidgetContainer< QString >
{
protected:
	typedef QString value_type;
	QLineEdit* lineEdit;

public:
	DataWidgetContainer() : lineEdit(nullptr) {}

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
Creator<DataWidgetFactory, SimpleDataWidget<int, CheckboxDataWidget> > DWClass_checkbox("checkbox",true);
Creator<DataWidgetFactory, SimpleDataWidget<int, SeedDataWidget> > DWClass_seed("seed",true);
Creator<DataWidgetFactory, SimpleDataWidget<double> > DWClass_double("default",true);
Creator<DataWidgetFactory, SimpleDataWidget<QString> > DWClass_string("default",true);
