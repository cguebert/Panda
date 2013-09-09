#ifndef STRUCTDATAWIDGET_H
#define STRUCTDATAWIDGET_H

#include <ui/DataWidget.h>

#include <QtWidgets>

class BaseDataWidgetDialog : public QDialog
{
public:
	virtual ~BaseDataWidgetDialog() {}
	virtual void readFromData() = 0;
	virtual void writeToData() = 0;
};

template<class T>
class TableDataDialog : public BaseDataWidgetDialog
{
public:
	typedef T value_type;
	typedef panda::Data<value_type> data_type;

	TableDataDialog(QWidget* parent, data_type* d)
		: data(d)
	{

	}

	virtual void readFromData()
	{

	}

	virtual void writeToData()
	{

	}

protected:
	data_type* data;
};

class BaseOpenDialogDataWidgetContainer : public QObject
{
	Q_OBJECT
signals:
	void editingFinished();

public slots:
	virtual void onShowDialog() {}
};

template<class T, class Dialog = TableDataDialog<T> >
class open_dialog_data_widget_container : public BaseOpenDialogDataWidgetContainer
{
public:
	typedef T value_type;
	typedef panda::Data<value_type> data_type;
	QWidget* container;
	Dialog* dialog;
	data_type* data;

	open_dialog_data_widget_container()
		: dialog(nullptr)
		, container(nullptr)
		, data(nullptr)
	{}

	QWidget* createWidgets(BaseDataWidget* parent, const data_type* d)
	{
		QWidget* container = new QWidget(parent);
		QPushButton* pushButton = new QPushButton("...");

		QHBoxLayout* layout = new QHBoxLayout(container);
		layout->setMargin(0);
		layout->addWidget(pushButton);
		container->setLayout(layout);

		QObject::connect(pushButton, SIGNAL(clicked()), this, SLOT(onShowDialog()) );
		QObject::connect(this, SIGNAL(editingFinished()), parent, SLOT(setWidgetDirty()) );

		return container;
	}

	void readFromData(const data_type* d)
	{
		if(dialog)
			dialog->readFromData();
	}

	void writeToData(data_type* d)
	{
		if(dialog)
			dialog->writeToData();
	}

	void onShowDialog()
	{
		if(!dialog)
			dialog = new Dialog(container, data);

		if(dialog->exec() == QDialog::Accepted)
		{
			dialog->writeToData();
			emit editingFinished();
		}
		else
			dialog->readFromData();	// Reseting dialog
	}
};

//***************************************************************//

#endif
