#ifndef LISTDATAWIDGETDIALOG_H
#define LISTDATAWIDGETDIALOG_H

#include <ui/DataWidget.h>
#include <ui/StructTraits.h>

#include <QtWidgets>

class BaseListDataWidgetDialog : public QDialog
{
	Q_OBJECT
public:
	BaseListDataWidgetDialog(QWidget* parent)
		: QDialog(parent)
	{}

signals:

public slots:
	virtual void refreshPreviews() {}
	virtual void resizeValue() {}
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

template<class T, class Container = DataWidgetContainer< VectorDataTrait<T>::row_type > >
class ListDataWidgetDialog : public BaseListDataWidgetDialog
{
protected:
	typedef T value_type;
	typedef panda::Data<T> data_type;
	typedef VectorDataTrait<value_type> row_trait;
	typedef typename row_trait::row_type row_type;

	typedef QSharedPointer<Container> ContainerPtr;

	bool readOnly;
	QScrollArea* scrollArea;
	QFormLayout* formLayout;
	QWidget* resizeWidget;
	QSpinBox* resizeSpinBox;
	QVector<ContainerPtr> containers;

public:
	ListDataWidgetDialog(QWidget* parent, bool readOnly, QString name)
		: BaseListDataWidgetDialog(parent)
		, readOnly(readOnly)
		, scrollArea(nullptr)
		, formLayout(nullptr)
		, resizeWidget(nullptr)
		, resizeSpinBox(nullptr)
	{
		QVBoxLayout* mainLayout = new QVBoxLayout();

		if(!readOnly)
		{
			resizeWidget = new QWidget(this);
			QLabel* resizeLabel = new QLabel(tr("Size"));
			resizeSpinBox = new QSpinBox();
			resizeSpinBox->setMinimum(0);
			resizeSpinBox->setMaximum(65535);
			QPushButton* resizeButton = new QPushButton(tr("Resize"));
			QHBoxLayout* resizeLayout = new QHBoxLayout;
			resizeLayout->setMargin(0);
			resizeLayout->addWidget(resizeLabel);
			resizeLayout->addWidget(resizeSpinBox, 1);
			resizeLayout->addWidget(resizeButton);
			resizeWidget->setLayout(resizeLayout);
			mainLayout->addWidget(resizeWidget);

			connect(resizeButton, SIGNAL(clicked()), this, SLOT(resizeValue()));
		}

		scrollArea = new QScrollArea(this);
		scrollArea->setFrameShape(QFrame::NoFrame);
		scrollArea->setWidgetResizable(true);
		QWidget *layoutWidget = new QWidget(scrollArea);
		formLayout = new QFormLayout(layoutWidget);
		formLayout->setMargin(0);
		formLayout->setSizeConstraint(QLayout::SetMinimumSize);
		scrollArea->setWidget(layoutWidget);
		mainLayout->addWidget(scrollArea);

		QPushButton* okButton = new QPushButton(tr("Ok"), this);
		okButton->setDefault(true);
		connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
		QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);
		connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
		QHBoxLayout* buttonsLayout = new QHBoxLayout;
		buttonsLayout->addStretch();
		buttonsLayout->addWidget(okButton);
		buttonsLayout->addWidget(cancelButton);
		mainLayout->addLayout(buttonsLayout);

		setLayout(mainLayout);
		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

		setWindowTitle(name + (readOnly ? tr(" (read-only)") : ""));
	}

	void updateTable(const value_type& v)
	{
		int nb = row_trait::size(v);

		for(int i = 0; i < nb; ++i)
		{
			const row_type* row = row_trait::get(v, i);
			if(row)
				containers[i]->readFromData(*row);
		}
	}

	virtual void readFromData(const value_type& v)
	{
		resizeContainers(row_trait::size(v));
		updateTable(v);

		if(resizeSpinBox)
			resizeSpinBox->setValue(containers.size());
	}

	value_type readFromTable()
	{
		value_type v;
		int nbRows = containers.size();
		row_trait::resize(v, nbRows);

		for(int i = 0; i < nbRows; ++i)
		{
			const row_type* row = row_trait::get(v, i);
			if(row)
			{
				row_type val;
				containers[i]->writeToData(val);
				row_trait::set(v, val, i);
			}
		}

		return v;
	}

	virtual void writeToData(value_type& v)
	{
		v = readFromTable();
	}

	void resizeContainers(int nb)
	{
		int oldSize = containers.size();
		if(oldSize == nb)
			return;

		if(oldSize > nb)
		{	// Removing
			containers.resize(nb);
			for(int i=oldSize-1; i>=nb; --i)
			{
				QLayoutItem* label = formLayout->itemAt(i, QFormLayout::LabelRole);
				QLayoutItem* field = formLayout->itemAt(i, QFormLayout::FieldRole);
				label->widget()->hide();
				field->widget()->hide();
				formLayout->removeItem(label);
				formLayout->removeItem(field);
				delete label;
				delete field;
			}
		}
		else
		{	// Adding
			for(int i=oldSize; i<nb; ++i)
			{
				ContainerPtr container = ContainerPtr(new Container());
				containers.push_back(container);
				QWidget* widget = container->createWidgets(this, readOnly);
				container->updatePreview();
				connect(container.data(), SIGNAL(editingFinished()), this, SLOT(refreshPreviews()));
				formLayout->addRow(QString::number(i), widget);
			}
		}
	}

	virtual void resizeValue()
	{
		int nb = resizeSpinBox->value();
		resizeContainers(nb);
	}

	virtual void refreshPreviews()
	{
		for(ContainerPtr container : containers)
			container->updatePreview();
	}
};

#endif // LISTDATAWIDGETDIALOG_H
