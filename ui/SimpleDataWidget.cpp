#include <ui/DataWidgetFactory.h>
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

template <class T>
class SliderDataWidget : public DataWidgetContainer< T >
{
protected:
	typedef T value_type;
	QSlider* slider;
	QLabel* label;
	QWidget* container;
	value_type vMin, vMax, step;
	int sRange;

public:
	SliderDataWidget() : slider(nullptr), label(nullptr), container(nullptr)
	  , vMin(0), vMax(100), step(1), sRange(100) {}

	QWidget* createWidgets(BaseDataWidget* parent, bool readOnly)
	{
		container = new QWidget(parent);

		label = new QLabel(container);

		slider = new QSlider(container);
		slider->setOrientation(Qt::Horizontal);
		slider->setTickInterval(QSlider::NoTicks);
		slider->setEnabled(!readOnly);
		readParameters(parent->getBaseData()->getWidgetData());

		QHBoxLayout* layout = new QHBoxLayout(container);
		layout->setMargin(0);
		layout->addWidget(label, 1);
		layout->addWidget(slider, 5);
		container->setLayout(layout);

		QObject::connect(slider, SIGNAL(valueChanged(int)), parent, SLOT(setWidgetDirty()));
		return container;
	}
	void readParameters(QString wd)
	{
		if(!wd.isEmpty())
		{
			QTextStream stream(&wd);
			stream >> vMin >> vMax >> step;

			if(vMin > vMax)
				std::swap(vMin, vMax);
			if(!vMin && !vMax) vMax = 100;
			step = abs(step);
			if(!step) step = 1;

			sRange = (vMax - vMin) / step;
		}

		slider->setMinimum(0);
		slider->setMaximum(sRange);
		slider->setSingleStep(1);
		slider->setPageStep(qMax(5, sRange/10));
	}
	void readFromData(const value_type& v)
	{
		value_type t = qBound(vMin, v, vMax);
		t = (t - vMin) / step;
		slider->setValue(t);
		label->setText(QString::number(v));
	}
	void writeToData(value_type& v)
	{
		value_type t = slider->value();
		v = t * step + vMin;
		label->setText(QString::number(v));
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

class EnumDataWidget
{
protected:
	typedef int value_type;
	QComboBox* comboBox;

public:
	EnumDataWidget() : comboBox(nullptr) {}

	QWidget* createWidgets(BaseDataWidget* parent, bool readOnly)
	{
		comboBox = new QComboBox(parent);
		comboBox->setEnabled(!readOnly);

		QString wd = parent->getBaseData()->getWidgetData();
		QStringList list = wd.split(";", QString::SkipEmptyParts);
		for(auto v : list)
			comboBox->addItem(v);

		QObject::connect(comboBox, SIGNAL(currentIndexChanged(int)), parent, SLOT(setWidgetDirty()));
		return comboBox;
	}
	void readFromData(const value_type& v)
	{
		comboBox->blockSignals(true);
		comboBox->setCurrentIndex(v);
		comboBox->blockSignals(false);
	}
	void writeToData(value_type& v)
	{
		v = comboBox->currentIndex();
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

RegisterWidget<SimpleDataWidget<int> > DWClass_int("default");
RegisterWidget<SimpleDataWidget<int, CheckboxDataWidget> > DWClass_checkbox("checkbox");
RegisterWidget<SimpleDataWidget<int, SeedDataWidget> > DWClass_seed("seed");
RegisterWidget<SimpleDataWidget<int, EnumDataWidget> > DWClass_enum("enum");
RegisterWidget<SimpleDataWidget<int, SliderDataWidget<int> > > DWClass_slider_int("slider");
RegisterWidget<SimpleDataWidget<double> > DWClass_double("default");
RegisterWidget<SimpleDataWidget<double, SliderDataWidget<double> > > DWClass_slider_double("slider");
RegisterWidget<SimpleDataWidget<QString> > DWClass_string("default");
