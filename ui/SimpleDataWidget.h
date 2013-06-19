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
class data_widget_trait
{
public:
    typedef T data_type;
    typedef QLineEdit Widget;
    static Widget* create(QWidget* parent, const data_type& /*d*/)
    {
        Widget* w = new Widget(parent);
        return w;
    }
    static void readFromData(Widget* w, const data_type& d)
    {
        QTextStream o;
        o << d;
        if (o.string() != w->text())
            w->setText(o.string());
    }
    static void writeToData(Widget* w, data_type& d)
    {
        QString s = w->text();
        QTextStream i(&s);
        i >> d;
    }
    static void connectChanged(Widget* w, BaseDataWidget* datawidget)
    {
        connect(w, SIGNAL(textChanged(const QString&)), datawidget, SLOT(setWidgetDirty()));
    }
};


/// This class is used to create and manage the GUI of a data type,
/// using data_widget_trait to know which widgets to use
template<class T>
class data_widget_container
{
public:
    typedef T data_type;
    typedef data_widget_trait<data_type> helper;
    typedef typename helper::Widget Widget;
    typedef QHBoxLayout Layout;
    Widget* w;
    Layout* container_layout;
    data_widget_container() : w(nullptr),container_layout(nullptr) {  }

    bool createLayout(BaseDataWidget* parent)
    {
        if(parent->layout() != nullptr) return false;
        container_layout = new QHBoxLayout(parent);
        return true;
    }

    bool createLayout(QLayout* layout)
    {
        if(container_layout) return false;
        container_layout = new QHBoxLayout(layout);
        return true;
    }

    bool createWidgets(BaseDataWidget* parent, const data_type& d, bool readOnly)
    {
        w = helper::create(parent,d);
        if (w == nullptr)
            return false;

        helper::readFromData(w, d);
        if (readOnly)
            w->setEnabled(false);
        else
            helper::connectChanged(w, parent);
        return true;
    }
    void setReadOnly(bool readOnly)
    {
        w->setEnabled(!readOnly);
    }
    void readFromData(const data_type& d)
    {
        helper::readFromData(w, d);
    }
    void writeToData(data_type& d)
    {
        helper::writeToData(w, d);
    }

    void insertWidgets()
    {
        Q_ASSERT(w);
        container_layout->addWidget(w);
    }
};

/// This class manages the GUI of a BaseData, using the corresponding instance of data_widget_container
template<class T, class Container = data_widget_container<T> >
class SimpleDataWidget : public DataWidget<T>
{
protected:
    typedef T data_type;
    Container container;
    typedef data_widget_trait<data_type> helper;

public:
    typedef panda::Data<T> MyTData;
    SimpleDataWidget(QWidget* parent, MyTData* d):
        DataWidget<T>(parent, d)
    {}

    virtual bool createWidgets()
    {
        const data_type& d = this->getData()->getValue();
        if (!container.createWidgets(this, d, getData()->isReadOnly()) )
            return false;

        container.createLayout(this);
        container.insertWidgets();
        return true;
    }
    virtual void readFromData()
    {
        container.readFromData(this->getData()->getValue());
    }

    virtual void setReadOnly(bool readOnly)
    {
        container.setReadOnly(readOnly);
    }

    virtual void writeToData()
    {
        data_type& d = *this->getData()->beginEdit();
        container.writeToData(d);
        this->getData()->endEdit();
    }
};

template<>
class data_widget_trait< int >
{
public:
    typedef int data_type;
    typedef QSpinBox Widget;
    static Widget* create(QWidget* parent, const data_type&)
    {
        Widget* w = new Widget(parent);
        w->setMinimum(INT_MIN);
        w->setMaximum(INT_MAX);
        w->setSingleStep(1);
        return w;
    }
    static void readFromData(Widget* w, const data_type& d)
    {
        if (d != w->value())
            w->setValue(d);
    }
    static void writeToData(Widget* w, data_type& d)
    {
        d = w->value();
    }
    static void connectChanged(Widget* w, BaseDataWidget* datawidget)
    {
        QObject::connect(w, SIGNAL(valueChanged(int)), datawidget, SLOT(setWidgetDirty()));
    }
};

class checkbox_data_widget_trait : public data_widget_trait< int >
{
public:
    typedef int data_type;
    typedef QCheckBox Widget;
    static Widget* create(QWidget* parent, const data_type&)
    {
        Widget* w = new Widget(parent);
        return w;
    }
    static void readFromData(Widget* w, const data_type& d)
    {
        bool b = (d!=0);
        if (w->isChecked() != b)
            w->setChecked(b);
    }
    static void writeToData(Widget* w, data_type& d)
    {
        d = w->isChecked() ? 1 : 0;
    }
    static void connectChanged(Widget* w, BaseDataWidget* datawidget)
    {
        QObject::connect(w, SIGNAL(toggled(bool)), datawidget, SLOT(setWidgetDirty()));
    }
};

template <>
class data_widget_trait< double >
{
public:
    typedef double data_type;
    typedef QLineEdit Widget;
    static Widget* create(QWidget* parent, const data_type& /*d*/)
    {
        Widget* w = new Widget(parent);
        w->setValidator(new QDoubleValidator(w));
        return w;
    }
    static void readFromData(Widget* w, const data_type& d)
    {
        if (d != w->text().toDouble())
            w->setText(QString::number(d));
    }
    static void writeToData(Widget* w, data_type& d)
    {
        d = w->text().toDouble();
    }
    static void connectChanged(Widget* w, BaseDataWidget* datawidget)
    {
        QObject::connect(w, SIGNAL(textChanged(const QString&)), datawidget, SLOT(setWidgetDirty()));
    }
};

template<>
class data_widget_trait < QString >
{
public:
    typedef QString data_type;
    typedef QLineEdit Widget;
    static Widget* create(QWidget* parent, const data_type&)
    {
        Widget* w = new Widget(parent);
        return w;
    }
    static void readFromData(Widget* w, const data_type& d)
    {
        if (w->text() != d)
            w->setText(d);
    }
    static void writeToData(Widget* w, data_type& d)
    {
        d = w->text();
    }
    static void connectChanged(Widget* w, BaseDataWidget* datawidget)
    {
        QObject::connect(w, SIGNAL(textChanged(const QString&)), datawidget, SLOT(setWidgetDirty()) );
    }
};

#endif
