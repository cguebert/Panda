#ifndef SIMPLEDATAWIDGET_H
#define SIMPLEDATAWIDGET_H

#include <ui/DataWidget.h>

class QLineEdit;
class QPushButton;

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

    QWidget* createWidgets(BaseDataWidget* parent, const data_type* /*d*/)
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
    SimpleDataWidget(QWidget* parent, MyTData* d) :
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

class DataWidgetColorChooser : public QWidget
{
    Q_OBJECT
protected:
    QPushButton* pushButton;
    QColor theColor;

public:
    DataWidgetColorChooser(QColor color);

    void setColor(QColor color) { theColor = color; }
    QColor getColor() { return theColor; }

signals:
    void colorEdited();

public slots:
    void onChooseColor();
};

#endif
