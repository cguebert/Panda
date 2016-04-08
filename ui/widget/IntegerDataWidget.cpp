#include <ui/widget/DataWidgetFactory.h>
#include <ui/widget/OpenDialogDataWidget.h>
#include <ui/widget/ListDataWidgetDialog.h>
#include <ui/widget/NumericalDataWidget.h>
#include <ui/widget/TableDataWidgetDialog.h>
#include <ui/custom/qxt/QxtCheckComboBox.h>

#include <QtWidgets>

#include <panda/helper/Random.h>

#include <panda/types/IntVector.h>
using panda::types::IntVector;

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

//****************************************************************************//

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
		QObject::connect(checkBox, SIGNAL(clicked(bool)), parent, SLOT(setWidgetDirty()));
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

//****************************************************************************//

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
		button->setEnabled(!readOnly);
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

//****************************************************************************//

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

		QString wd = parent->getParameters();
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
	static QString GetParametersFormat()
	{
		return "val1;val2;val3;...";
	}
};

//****************************************************************************//

class FlagsDataWidget
{
protected:
	typedef int value_type;
	QxtCheckComboBox* comboBox;
	int nbFlags = 0;

public:
	FlagsDataWidget() : comboBox(nullptr) {}

	QWidget* createWidgets(BaseDataWidget* parent, bool readOnly)
	{
		comboBox = new QxtCheckComboBox(parent);
		comboBox->setDefaultText("No selection");
		comboBox->setDisplayMultipleSelection(false);
		comboBox->setMultipleSelectionText("%1 selected");
		comboBox->setEnabled(!readOnly);

		QString wd = parent->getParameters();
		QStringList list = wd.split(";", QString::SkipEmptyParts);
		nbFlags = list.size();
		for(auto v : list)
			comboBox->addItem(v);

		QObject::connect(comboBox, SIGNAL(checkedItemsChanged(QStringList)), parent, SLOT(setWidgetDirty()));
		return comboBox;
	}
	void readFromData(const value_type& v)
	{
		comboBox->blockSignals(true);
		for (int i = 0; i < nbFlags; ++i)
			comboBox->setItemCheckState(i, (v & (1 << i)) ? Qt::Checked : Qt::Unchecked);
		comboBox->blockSignals(false);
	}
	void writeToData(value_type& v)
	{
		int val = 0;
		for (int i = 0; i < nbFlags; ++i)
		{
			if (comboBox->itemCheckState(i) == Qt::Checked)
				val += 1 << i;
		}
		v = val;
	}
	static QString GetParametersFormat()
	{
		return "val1;val2;val3;...";
	}
};

//****************************************************************************//

template<>
static QString FlatDataTrait<int>::toString(const value_type& d)
{ return QString::number(d); }

template<>
class VectorDataTrait<IntVector>
{
public:
	typedef IntVector vector_type;
	typedef int row_type;

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

RegisterWidget<SimpleDataWidget<int> > DWClass_int("default");
RegisterWidget<SimpleDataWidget<int, CheckboxDataWidget> > DWClass_checkbox("checkbox");
RegisterWidget<SimpleDataWidget<int, SeedDataWidget> > DWClass_seed("seed");
RegisterWidget<SimpleDataWidget<int, EnumDataWidget> > DWClass_enum("enum");
RegisterWidget<SimpleDataWidget<int, FlagsDataWidget> > DWClass_flags("flags");
RegisterWidget<SimpleDataWidget<int, SliderDataWidget<int> > > DWClass_slider_int("slider");
RegisterWidget<OpenDialogDataWidget<std::vector<int>, ListDataWidgetDialog<std::vector<int> > > > DWClass_ints_list_generic("generic");

RegisterWidget<OpenDialogDataWidget<IntVector, TableDataWidgetDialog<IntVector> > > DWClass_strings_list("default");
RegisterWidget<OpenDialogDataWidget<std::vector<IntVector>, ListDataWidgetDialog<std::vector<IntVector> > > > DWClass_paths_list_generic("generic");
