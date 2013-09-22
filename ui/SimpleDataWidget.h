#ifndef SIMPLEDATAWIDGET_H
#define SIMPLEDATAWIDGET_H

#include <ui/DataWidget.h>
#include <panda/DataTraits.h>

class QLineEdit;
class QPushButton;

/// This class is used to specify how to graphically represent a data type,
/// by default using a simple QLineEdit
template<class T>
class data_widget_container
{
protected:
	typedef T value_type;
	typedef panda::Data<T> data_type;
	typedef panda::data_trait<T> trait;
	QLineEdit* lineEdit;

public:
    data_widget_container() : lineEdit(nullptr) {}

	QWidget* createWidgets(BaseDataWidget* parent, bool readOnly)
    {
		lineEdit = new QLineEdit(parent);
		lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		lineEdit->setEnabled(!readOnly);
		QObject::connect(lineEdit, SIGNAL(editingFinished()), parent, SLOT(setWidgetDirty()));
		return lineEdit;
    }

	void readFromData(const value_type& d)
    {
		QString s = panda::valueToString(d);
		if(s != lineEdit->text())
			lineEdit->setText(s);
    }

	void writeToData(value_type& d)
    {
		QString s = lineEdit->text();
		d = panda::valueFromString<value_type>(s);
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
    SimpleDataWidget(QWidget* parent, MyTData* d) :
        DataWidget<T>(parent, d)
    {}

	virtual QWidget* createWidgets(bool readOnly)
    {
		QWidget* w = container.createWidgets(this, readOnly);
		if(!w)
			return nullptr;

		container.readFromData(getData()->getValue());
		return w;
    }

    virtual void readFromData()
    {
		container.readFromData(getData()->getValue());
    }

    virtual void writeToData()
    {
		MyTData* data = getData();
		value_type& v = *data->beginEdit();
		container.writeToData(v);
		data->endEdit();
    }
};

#endif
