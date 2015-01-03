#ifndef SIMPLEDATAWIDGET_H
#define SIMPLEDATAWIDGET_H

#include <ui/widget/DataWidget.h>
#include <ui/widget/StructTraits.h>
#include <panda/types/DataTraits.h>

class QLineEdit;
class QPushButton;
class QLabel;

/// This class is used to specify how to graphically represent a data type,
template<class T>
class DataWidgetContainer
{
protected:
	typedef T value_type;
	typedef panda::Data<T> data_type;
	typedef panda::types::DataTrait<value_type> trait;

public:
	DataWidgetContainer() {}

	QWidget* createWidgets(BaseDataWidget* parent, bool readOnly);
	void readFromData(const value_type& d);
	void writeToData(value_type& d);
};

//****************************************************************************//

/// This class manages the GUI of a BaseData, using the corresponding instance of DataWidgetContainer
template<class T, class Container = DataWidgetContainer<T> >
class SimpleDataWidget : public DataWidget<T>
{
protected:
	typedef T value_type;
	Container container;

public:
	typedef panda::Data<T> TData;
	SimpleDataWidget(QWidget* parent, TData* d)
		: DataWidget<T>(parent, d)
	{}

	SimpleDataWidget(QWidget* parent, value_type* pValue, QString widgetName, QString name, QString parameters)
		: DataWidget<T>(parent, pValue, widgetName, name, parameters)
	{}

	virtual QWidget* createWidgets(bool readOnly)
	{
		QWidget* w = container.createWidgets(this, readOnly);
		if(!w)
			return nullptr;

		container.readFromData(getValue());
		return w;
	}

	virtual void readFromData()
	{
		container.readFromData(getValue());
	}

	virtual void writeToData()
	{
		value_type value = getValue();
		container.writeToData(value);
		setValue(value);
	}

	static QString GetParametersFormat() { return ParametersFormatHelper<Container>(); }
};

#endif // SIMPLEDATAWIDGET_H
