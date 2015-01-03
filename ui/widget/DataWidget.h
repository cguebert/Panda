#ifndef DATAWIDGET_H
#define DATAWIDGET_H

#include <panda/Data.h>
#include <panda/PandaDocument.h>
#include <panda/helper/Factory.h>

#include <ui/command/SetDataValueCommand.h>

#include <QWidget>
#include <QString>

#include <type_traits>

// Hack that should work with the main compilers (it works on VC12)
// Test if a class has the member function GetParametersFormat, and call it
namespace
{
template<class> struct sfinae_true : std::true_type{};
template<class T> static auto testGPF(int) -> sfinae_true<decltype(T::GetParametersFormat)>;
template<class> static auto testGPF(long) -> std::false_type;
template<class T> struct hasGPF : decltype(testGPF<T>(0)){}; // the '0' is to use the first version (int) if possible
}

template<typename T>
typename std::enable_if_t<::hasGPF<T>::value, QString> ParametersFormatHelper()
{ return T::GetParametersFormat(); }

template<typename T>
typename std::enable_if_t<!::hasGPF<T>::value, QString> ParametersFormatHelper()
{ return ""; }

/**
*\brief Abstract Interface of a QWidget which allows to edit a data.
*/
class BaseDataWidget : public QWidget
{
	Q_OBJECT
public:
	BaseDataWidget(QWidget* parent, QString widgetName, QString name, QString parameters)
		: QWidget(parent)
		, m_widgetName(widgetName)
		, m_displayName(name)
		, m_parameters(parameters)
		, m_dirty(false)
		, m_counter(-1)
	{ }

	virtual ~BaseDataWidget() {}

	QString getDisplayName() { return m_displayName; }
	QString getParameters() { return m_parameters; }
	QString getWidgetName() { return m_widgetName; }
	virtual int getCounter() { return -1; }	// If <0, always update

	bool isDirty() { return m_dirty; }

	/// The implementation of this method holds the widget creation and the signal / slot connections.
	virtual QWidget* createWidgets(bool readOnly = true) = 0;

public slots:
	/// Checks that widget has been edited
	void updateDataValue()
	{
		if(m_dirty)
			writeToData();

		m_dirty = false;
		m_counter = getCounter();
	}

	/// First checks that the widget is not currently being edited
	/// checks that the data has changed since the last time the widget
	/// has read the data value.
	/// ultimately read the data value.
	void updateWidgetValue()
	{
		if(!m_dirty)
		{
			int newCounter = getCounter();
			if(m_counter != newCounter || newCounter < 0)
			{
				readFromData();
				m_counter = newCounter;
				update();
			}
		}
	}
	/// You call this slot anytime you want to specify that the widget
	/// value is out of sync with the underlying data value.
	void setWidgetDirty(bool b = true)
	{
		m_dirty = b;
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

	QString m_widgetName, m_displayName, m_parameters;
	bool m_dirty;
	int m_counter;
};

/**
*\brief This class is basically the same as DataWidget, except that it
* takes a template parameter so the actual type of the value is known.
* It can be read via getValue and set with setValue.
**/
template<class T>
class DataWidget : public BaseDataWidget
{
public:
	typedef panda::Data<T> TData;
	typedef typename TData::value_type value_type;
	typedef typename TData::const_reference const_reference;

	DataWidget(QWidget* parent, TData* d)
		: BaseDataWidget(parent, d->getWidget(), d->getName(), d->getWidgetData())
		, m_data(d)
		, m_value(nullptr)
	{}

	DataWidget(QWidget* parent, value_type* pValue, QString widgetName, QString name, QString parameters)
		: BaseDataWidget(parent, widgetName, name, parameters)
		, m_data(nullptr)
		, m_value(pValue)
	{}

	const_reference getValue()
	{
		if(m_data)
			return m_data->getValue();
		else
			return *m_value;
	}

	void setValue(const_reference value)
	{
		if(m_data)
		{
			auto oldValue = m_data->getValue();
			if(oldValue != value)
			{
				if(m_data->getOwner())
					m_data->getOwner()->getParentDocument()->addCommand(new SetDataValueCommand<T>(m_data, oldValue, value));
				else
					m_data->setValue(value);
			}
		}
		else
			*m_value = value;
	}

	int getCounter()
	{
		if(m_data)
			return m_data->getCounter();
		else
			return -1;
	}

	void changeDataPointer(TData* newData)
	{
		if(newData != m_data)
		{
			m_data = newData;
			m_value = nullptr;
			updateWidgetValue();
		}
	}

	void changeValuePointer(value_type* newValue)
	{
		if(newValue != m_value)
		{
			m_data = nullptr;
			m_value = newValue;
			updateWidgetValue();
		}
	}

protected:
	TData* m_data;
	value_type* m_value;
};

#endif // DATAWIDGET_H

