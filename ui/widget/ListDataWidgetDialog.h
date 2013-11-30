#ifndef LISTDATAWIDGETDIALOG_H
#define LISTDATAWIDGETDIALOG_H

#include <ui/widget/DataWidget.h>
#include <ui/widget/DataWidgetFactory.h>
#include <ui/widget/StructTraits.h>
#include <panda/types/DataTraits.h>

#include <QtWidgets>

class BaseListDataWidgetDialog : public QDialog
{
	Q_OBJECT
public:
	BaseListDataWidgetDialog(QWidget* parent)
		: QDialog(parent)
	{}

public slots:
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

template<class T>
class ListDataWidgetDialog : public BaseListDataWidgetDialog
{
protected:
	typedef T value_type;
	typedef panda::Data<T> data_type;
	typedef VectorDataTrait<value_type> row_trait;
	typedef typename row_trait::row_type row_type;

	typedef DataWidget<row_type> ChildDataWidget;
	typedef QSharedPointer<ChildDataWidget> DataWidgetPtr;

	bool readOnly;
	QScrollArea* scrollArea;
	QFormLayout* formLayout;
	QWidget* resizeWidget;
	QSpinBox* resizeSpinBox;
	DataWidget<value_type>* parentDW;
	value_type copyValue;
	QVector<DataWidgetPtr> dataWidgets;
	int rowTypeFullId;
	const BaseDataWidgetCreator* dataWidgetCreator;

public:
	ListDataWidgetDialog(DataWidget<value_type>* parent, bool readOnly, QString name)
		: BaseListDataWidgetDialog(parent)
		, readOnly(readOnly)
		, scrollArea(nullptr)
		, formLayout(nullptr)
		, resizeWidget(nullptr)
		, resizeSpinBox(nullptr)
		, parentDW(parent)
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

		rowTypeFullId = panda::types::DataTrait<row_type>::fullTypeId();
		dataWidgetCreator = DataWidgetFactory::getInstance()->getCreator(rowTypeFullId, parentDW->getWidgetName());
	}

	virtual void readFromData(const value_type& v)
	{
		copyValue = v;
		resize(row_trait::size(v));

		for(auto child : dataWidgets)
			child->updateWidgetValue();

		if(resizeSpinBox)
			resizeSpinBox->setValue(dataWidgets.size());
	}

	virtual void writeToData(value_type& v)
	{
		for(auto child : dataWidgets)
			child->updateDataValue();

		v = copyValue;
	}

	void resize(int nb)
	{
		int oldSize = dataWidgets.size();
		if(oldSize == nb)
			return;

		row_trait::resize(copyValue, nb);

		// Update value pointers if they changed
		int updateNb = qMin(nb, oldSize);
		for(int i=0; i<updateNb; ++i)
			dataWidgets[i]->changeValuePointer(row_trait::get(copyValue, i));

		if(oldSize > nb)
		{	// Removing
			dataWidgets.resize(nb);
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
		else if(dataWidgetCreator)
		{	// Adding
			QString widget = parentDW->getWidgetName(),
					parentName = parentDW->getDisplayName(),
					parameters = parentDW->getParameters();
			for(int i=oldSize; i<nb; ++i)
			{
				QString displayName = parentName + " / " + QString::number(i);
				row_type* pValue = row_trait::get(copyValue, i);
				BaseDataWidget* baseDataWidget = DataWidgetFactory::getInstance()
						->create(this, pValue, rowTypeFullId, widget, displayName, parameters);
				ChildDataWidget* dataWidget = dynamic_cast<ChildDataWidget*>(baseDataWidget);
				if(dataWidget)
				{
					dataWidgets.push_back(DataWidgetPtr(dataWidget));
					QWidget* widget = dataWidget->createWidgets(readOnly);
					formLayout->addRow(QString::number(i), widget);
				}
				else
					delete baseDataWidget;
			}
		}
	}

	virtual void resizeValue()
	{
		int nb = resizeSpinBox->value();
		resize(nb);
	}
};

#endif // LISTDATAWIDGETDIALOG_H
