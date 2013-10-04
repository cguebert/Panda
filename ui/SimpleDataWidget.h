#ifndef SIMPLEDATAWIDGET_H
#define SIMPLEDATAWIDGET_H

#include <ui/DataWidget.h>
#include <ui/StructTraits.h>

class QLineEdit;
class QPushButton;
class QLabel;

/// This class is used to specify how to graphically represent a data type,
/// by default using a simple QLineEdit
template<class T>
class DataWidgetContainer
{
protected:
	typedef T value_type;
	typedef panda::Data<T> data_type;
	typedef panda::DataTrait<value_type> trait;
	QLineEdit* lineEdit;

public:
	DataWidgetContainer() : lineEdit(nullptr) {}

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

/// This class manages the GUI of a BaseData, using the corresponding instance of DataWidgetContainer
template<class T, class Container = DataWidgetContainer<T> >
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
		auto v = getData()->getAccessor();
		container.writeToData(v.wref());
	}
};

//***************************************************************//

class BaseOpenDialogObject : public QObject
{
	Q_OBJECT
signals:
	void editingFinished();

public slots:
	virtual void onShowDialog() {}
};

template<class T, class Dialog >
class OpenDialogDataWidget : public DataWidget<T>, public BaseOpenDialogObject
{
protected:
	typedef T value_type;
	typedef panda::Data<T> data_type;

	QWidget* container;
	QLabel* label;
	Dialog* dialog;
	bool isReadOnly;

public:
	OpenDialogDataWidget(QWidget* parent, MyTData* d)
		: DataWidget<T>(parent, d)
		, dialog(nullptr)
		, container(nullptr)
		, label(nullptr)
		, isReadOnly(false)
	{}

	virtual QWidget* createWidgets(bool readOnly)
	{
		isReadOnly = readOnly;
		container = new QWidget(this);

		label = new QLabel("toto");
		label->setEnabled(!readOnly);

		QPushButton* pushButton = new QPushButton("...");
		pushButton->setMaximumWidth(40);

		QHBoxLayout* layout = new QHBoxLayout(container);
		layout->setMargin(0);
		layout->addWidget(label, 1);
		layout->addWidget(pushButton);
		container->setLayout(layout);

		BaseOpenDialogObject* boddw = dynamic_cast<BaseOpenDialogObject*>(this);
		BaseDataWidget* bdw = dynamic_cast<BaseDataWidget*>(this);
		QObject::connect(pushButton, SIGNAL(clicked()), boddw, SLOT(onShowDialog()) );
		QObject::connect(boddw, SIGNAL(editingFinished()), bdw, SLOT(setWidgetDirty()) );

		readFromData();

		return container;
	}

	virtual void readFromData()
	{
		updatePreview();
		if(dialog)
			dialog->readFromData(getData()->getValue());
	}

	virtual void writeToData()
	{
		if(dialog)
		{
			auto v = getData()->getAccessor();
			dialog->writeToData(v.wref());

			updatePreview();
		}
	}

	virtual void onShowDialog()
	{
		if(!dialog)
			dialog = new Dialog(container, *getData(), isReadOnly);

		dialog->readFromData(getData()->getValue());
		if(dialog->exec() == QDialog::Accepted && !isReadOnly)
			emit editingFinished();
	}

	void updatePreview()
	{
		const value_type& v = getData()->getValue();
		typedef VectorDataTrait<value_type> vector_trait;
		if(vector_trait::is_vector)
		{
			QString text = QString(container->tr("<i>%1 elements</i>")).arg(vector_trait::size(v));
			label->setText(text);
		}
		else
			label->setText(panda::valueToString(v));
	}
};

#endif
