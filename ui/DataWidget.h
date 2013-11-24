#ifndef DATAWIDGET_H
#define DATAWIDGET_H

#include <panda/Data.h>
#include <panda/PandaObject.h>
#include <panda/helper/Factory.h>

#include <QWidget>
#include <QString>

/**
*\brief Abstract Interface of a QWidget which allows to edit a data.
*/
class BaseDataWidget : public QWidget
{
	Q_OBJECT
public:
	BaseDataWidget(QWidget* parent, QString name, QString parameters)
		: QWidget(parent)
		, m_name(name)
		, m_parameters(parameters)
		, dirty(false)
		, counter(-1)
	{ }

	virtual ~BaseDataWidget() {}

	QString getName() { return m_name; }
	QString getParameters() { return m_parameters; }
	virtual int getCounter() { return -1; }	// If <0, always update

	bool isDirty() { return dirty; }

	/// The implementation of this method holds the widget creation and the signal / slot connections.
	virtual QWidget* createWidgets(bool readOnly = true) = 0;

public slots:
	/// Checks that widget has been edited
	void updateDataValue()
	{
		if(dirty)
			writeToData();

		dirty = false;
		counter = getCounter();
	}

	/// First checks that the widget is not currently being edited
	/// checks that the data has changed since the last time the widget
	/// has read the data value.
	/// ultimately read the data value.
	void updateWidgetValue()
	{
		if(!dirty)
		{
			int newCounter = getCounter();
			if(counter != newCounter || newCounter < 0)
			{
				readFromData();
				this->update();
			}
		}
	}
	/// You call this slot anytime you want to specify that the widget
	/// value is out of sync with the underlying data value.
	void setWidgetDirty(bool b = true)
	{
		dirty = b;
		updateDataValue();
		emit WidgetDirty(b);
	}

signals:
	/// Emitted each time setWidgetDirty is called. You can also emit
	/// it if you want to tell the widget value is out of sync with
	/// the underlying data value.
	void WidgetDirty(bool);

protected:
	/// The implementation of this method tells how the widget reads the value of the data.
	virtual void readFromData() = 0;
	/// The implementation of this methods needs to tell how the widget can write its value in the data
	virtual void writeToData() = 0;

	QString m_name, m_parameters;
	bool dirty;
	int counter;
};

/**
*\brief This class is basically the same as DataWidget, except that it
* takes a template parameter so the actual type of the value is known.
* It can be read via getValue and read with getAccessor.
**/
template<class T>
class DataWidget : public BaseDataWidget
{
public:
	typedef panda::Data<T> TData;
	typedef typename TData::const_reference const_reference;
	typedef typename TData::data_accessor accessor;

	DataWidget(QWidget* parent, TData* d)
		: BaseDataWidget(parent, d->getName(), d->getWidgetData())
		, data(d)
	{}

	const_reference getValue()
	{ return data->getValue(); }

	accessor getAccessor()
	{ return data->getAccessor(); }

	int getCounter()
	{ return data->getCounter(); }

protected:
	TData* data;
};

#endif // DATAWIDGET_H

