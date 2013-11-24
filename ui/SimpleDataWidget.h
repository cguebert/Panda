#ifndef SIMPLEDATAWIDGET_H
#define SIMPLEDATAWIDGET_H

#include <ui/DataWidget.h>
#include <panda/types/DataTraits.h>
#include <ui/StructTraits.h>

#include <QComboBox>

class QLineEdit;
class QPushButton;
class QLabel;
class QComboBox;

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

//***************************************************************//

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

	SimpleDataWidget(QWidget* parent, value_type* pValue, QString name, QString parameters)
		: DataWidget<T>(parent, pValue, name, parameters)
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
	OpenDialogDataWidget(QWidget* parent, TData* d)
		: DataWidget<T>(parent, d)
		, dialog(nullptr)
		, container(nullptr)
		, label(nullptr)
		, isReadOnly(false)
	{}

	OpenDialogDataWidget(QWidget* parent, value_type* pValue, QString name, QString parameters)
		: DataWidget<T>(parent, pValue, name, parameters)
		, dialog(nullptr)
		, container(nullptr)
		, label(nullptr)
		, isReadOnly(false)
	{}

	virtual QWidget* createWidgets(bool readOnly)
	{
		isReadOnly = readOnly;
		container = new QWidget(this);

		label = new QLabel("    ");
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
			dialog->readFromData(getValue());
	}

	virtual void writeToData()
	{
		if(dialog)
		{
			value_type value = getValue();
			dialog->writeToData(value);
			setValue(value);

			updatePreview();
		}
	}

	virtual void onShowDialog()
	{
		if(!dialog)
			dialog = new Dialog(container, isReadOnly, getName());

		dialog->readFromData(getValue());
		if(dialog->exec() == QDialog::Accepted && !isReadOnly)
			emit editingFinished();
	}

	void updatePreview()
	{
		const value_type& v = getValue();
		typedef VectorDataTrait<value_type> vector_trait;
		if(vector_trait::is_vector)
		{
			QString text = QString(container->tr("<i>%1 elements</i>")).arg(vector_trait::size(v));
			label->setText(text);
		}
		else
		{
			const vector_trait::row_type* row = vector_trait::get(v, 0);
			if(row)
			{
				typedef FlatDataTrait<vector_trait::row_type> item_trait;
				label->setText(item_trait::toString(*row));
			}
		}
	}
};

#endif
