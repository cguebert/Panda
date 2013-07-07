#ifndef SIMPLEDATAWIDGET_H
#define SIMPLEDATAWIDGET_H

#include <ui/DataWidget.h>

#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QDoubleValidator>

/// This class is used to specify how to graphically represent a data type,
/// by default using a simple QLineEdit
template<class T>
class data_widget_container
{
public:
	typedef T value_type;
	typedef panda::Data<T> data_type;
	QLineEdit* lineEdit;

	data_widget_container() : lineEdit(nullptr) {}

	QWidget* createWidgets(QWidget* parent, const data_type* /*d*/)
    {
		lineEdit = new QLineEdit(parent);
		lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		QObject::connect(lineEdit, SIGNAL(textChanged(const QString&)), parent, SLOT(setWidgetDirty()));
		return lineEdit;
    }

	void readFromData(const data_type* d)
    {
		QString s = d->toString();
		if(s != lineEdit->text())
			lineEdit->setText(s);
    }

	void writeToData(data_type* d)
    {
		QString s = lineEdit->text();
		d->fromString(s);
    }
};

//***************************************************************//

/// This class manages the GUI of a BaseData, using the corresponding instance of data_widget_container
template<class T, class Container = data_widget_container<T> >
class SimpleDataWidget : public DataWidget<T>
{
protected:
	typedef T value_type;
	Container container;

public:
    typedef panda::Data<T> MyTData;
    SimpleDataWidget(QWidget* parent, MyTData* d):
        DataWidget<T>(parent, d)
    {}

	virtual QWidget* createWidgets()
    {
		QWidget* w = container.createWidgets(this, this->getData());
		if(!w)
			return nullptr;

		container.readFromData(this->getData());
		return w;
    }

    virtual void readFromData()
    {
		container.readFromData(this->getData());
    }

    virtual void writeToData()
    {
		container.writeToData(this->getData());
    }
};

//***************************************************************//

template<>
class data_widget_container< int >
{
public:
	typedef int value_type;
	typedef panda::Data<value_type> data_type;
	QSpinBox* spinBox;

	data_widget_container() : spinBox(nullptr) {}

	QWidget* createWidgets(QWidget* parent, const data_type*)
    {
		spinBox = new QSpinBox(parent);
		spinBox->setMinimum(INT_MIN);
		spinBox->setMaximum(INT_MAX);
		spinBox->setSingleStep(1);

		QObject::connect(spinBox, SIGNAL(valueChanged(int)), parent, SLOT(setWidgetDirty()));

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

	QWidget* createWidgets(QWidget* parent, const data_type*)
    {
		checkBox = new QCheckBox(parent);
		QObject::connect(checkBox, SIGNAL(toggled(bool)), parent, SLOT(setWidgetDirty()));
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

	QWidget* createWidgets(QWidget* parent, const data_type* /*d*/)
    {
		lineEdit = new QLineEdit(parent);
		lineEdit->setValidator(new QDoubleValidator(lineEdit));
		QObject::connect(lineEdit, SIGNAL(textChanged(const QString&)), parent, SLOT(setWidgetDirty()));
		return lineEdit;
    }
	void readFromData(const data_type* d)
    {
		value_type v = d->getValue();
		if (v != lineEdit->text().toDouble())
			lineEdit->setText(QString::number(v));
    }
	void writeToData(data_type* d)
    {
		d->setValue(lineEdit->text().toDouble());
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

	QWidget* createWidgets(QWidget* parent, const data_type*)
    {
		lineEdit = new QLineEdit(parent);
		QObject::connect(lineEdit, SIGNAL(textChanged(const QString&)), parent, SLOT(setWidgetDirty()) );
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

#endif
