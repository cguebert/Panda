#ifndef DATAWIDGET_H
#define DATAWIDGET_H

#include <panda/Data.h>
#include <panda/PandaObject.h>
#include <helper/Factory.h>

#include <QWidget>
#include <QString>

/**
*\brief Abstract Interface of a qwidget which allows to edit a data.
*/
class BaseDataWidget : public QWidget
{
    Q_OBJECT
public:
    struct CreatorArgument
    {
        panda::BaseData* data;
        QWidget* parent;
    };

    typedef panda::helper::Factory<BaseDataWidget, BaseDataWidget::CreatorArgument> DataWidgetFactory;

    template<class T>
    static T* create(T*, const BaseDataWidget::CreatorArgument& arg)
    {
        typename T::MyData* data = dynamic_cast<typename T::MyData*>(arg.data);
        if(!data)
            return nullptr;
        return new T(arg.parent, data);
    }

    static BaseDataWidget* CreateDataWidget(const BaseDataWidget::CreatorArgument &arg)
    {
        BaseDataWidget *dataWidget = nullptr;
        const QString &widgetName = arg.data->getWidget();
        if (widgetName.isEmpty())
            dataWidget = DataWidgetFactory::CreateAnyObject(arg);
        else
            dataWidget = DataWidgetFactory::CreateObject(widgetName, arg);

        return dataWidget;
    }

public slots:
    /// Checks that widget has been edited
    void updateDataValue()
    {
        if(dirty)
            writeToData();

        dirty = false;
		counter = baseData->getCounter();
    }

    /// First checks that the widget is not currently being edited
    /// checks that the data has changed since the last time the widget
    /// has read the data value.
    /// ultimately read the data value.
    void updateWidgetValue()
    {
        if(!dirty)
        {
            if(counter != baseData->getCounter())
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

public:
    typedef panda::BaseData MyData;

    BaseDataWidget(QWidget* parent, MyData* d)
        : QWidget(parent)
        , baseData(d)
        , dirty(false)
        , counter(-1)
    {
    }

    virtual ~BaseDataWidget()
    {
    }

    inline virtual void setData(MyData* d)
    {
        baseData = d;
        readFromData();
    }

    /// BaseData pointer accessor function.
    const panda::BaseData* getBaseData() const
    {
        return baseData;
    }

    panda::BaseData* getBaseData()
    {
        return baseData;
    }

    bool isDirty()
    {
        return dirty;
    }

	virtual void setWidgetEnabled(QWidget* /*widget*/, bool /*enable*/ = true)
	{
	}

    /// The implementation of this method holds the widget creation and the signal / slot
    /// connections.
	virtual QWidget* createWidgets(bool readOnly = true) = 0;

protected:
    /// The implementation of this method tells how the widget reads the value of the data.
    virtual void readFromData() = 0;
    /// The implementation of this methods needs to tell how the widget can write its value
    /// in the data
    virtual void writeToData() = 0;

    panda::BaseData* baseData;
    bool dirty;
    int counter;
};

/**
*\brief This class is basically the same as DataWidget, except that it
* takes a template parameter so the actual type of Data can be retrieved
* through the getData() accessor. In most cases you will need to derive
* from this class to implement the edition of your data in the GUI.
**/
template<class T>
class DataWidget : public BaseDataWidget
{
public:
    typedef panda::Data<T> MyTData;

    DataWidget(QWidget* parent, MyTData* d)
        : BaseDataWidget(parent, d)
        , Tdata(d)
    {
    }

    template <class RealObject>
    static RealObject* create(RealObject*, CreatorArgument& arg)
    {
        typename RealObject::MyTData* realData = dynamic_cast<typename RealObject::MyTData*>(arg.data);
        if(!realData)
            return nullptr;
        else
            return new RealObject(arg.parent, realData);
    }

    panda::Data<T>* getData()
    {
        return Tdata;
    }

    const panda::Data<T>* getData() const
    {
        return Tdata;
    }

    inline virtual void setData(MyTData* d)
    {
        Tdata = d;
    }

protected:
    MyTData* Tdata;
};

typedef panda::helper::Factory<BaseDataWidget, BaseDataWidget::CreatorArgument> DataWidgetFactory;

#endif // DATAWIDGET_H

