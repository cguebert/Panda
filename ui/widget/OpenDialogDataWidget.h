#ifndef OPENDIALOGDATAWIDGET_H
#define OPENDIALOGDATAWIDGET_H

#include <ui/widget/DataWidget.h>
#include <ui/widget/StructTraits.h>

#include <QStylePainter>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

// For the more complex widgets that need to open a dialog
class BaseOpenDialogObject : public QObject
{
	Q_OBJECT
signals:
	void editingFinished();

public slots:
	virtual void onShowDialog() {}
};

class ObjectWithPreview
{
public:
	virtual void draw(QPainter& painter, QSize size) = 0;
};

class PreviewView : public QWidget
{
public:
	PreviewView(ObjectWithPreview* obj) : object(obj) {}
	void paintEvent(QPaintEvent*)
	{
		QStylePainter painter(this);
		object->draw(painter, size());
	}

protected:
	ObjectWithPreview* object;
};

template<class T, class Dialog >
class OpenDialogDataWidget : public DataWidget<T>, public BaseOpenDialogObject
{
protected:
	typedef T value_type;
	typedef panda::Data<T> data_type;

	QWidget* container;
	QLabel* label;
	QPushButton* pushButton;
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

	OpenDialogDataWidget(QWidget* parent, value_type* pValue, QString widgetName, QString name, QString parameters)
		: DataWidget<T>(parent, pValue, widgetName, name, parameters)
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

		pushButton = new QPushButton("...");
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
			dialog = new Dialog(this, isReadOnly, getDisplayName());

		dialog->readFromData(getValue());
		if(dialog->exec() == QDialog::Accepted && !isReadOnly)
			emit editingFinished();
	}

	QString itemToString(const value_type& v, int index)
	{
		using vector_trait = VectorDataTrait<value_type>;
		using item_trait = FlatDataTrait<typename vector_trait::row_type>;
		return item_trait::toString(vector_trait::get(v, index));
	}

	void updatePreview()
	{
		const value_type& v = getValue();
		using vector_trait = VectorDataTrait<value_type>;
		if(vector_trait::is_vector)
		{
			int size = vector_trait::size(v);
			QString text;
			if (size == 1)
			{
				text = itemToString(v, 0).trimmed();
				if (text.isEmpty())
					text = container->tr("<i>1 element</i>").arg(size);
				else
					text = container->tr("<i>1 element: </i>&nbsp;&nbsp;%1").arg(text);
			}
			else
				text = container->tr("<i>%1 elements</i>").arg(size);
			label->setText(text);

			if(isReadOnly)
				pushButton->setEnabled(size != 0);
			else
				pushButton->setEnabled(true);
		}
		else
		{
			
			label->setText(itemToString(v, 0));
		}
	}

	static QString GetParametersFormat() { return ParametersFormatHelper<Dialog>(); }
};

#endif // OPENDIALOGDATAWIDGET_H
