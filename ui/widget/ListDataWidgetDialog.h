#ifndef LISTDATAWIDGETDIALOG_H
#define LISTDATAWIDGETDIALOG_H

#include <ui/widget/DataWidget.h>
#include <ui/widget/DataWidgetFactory.h>
#include <ui/widget/StructTraits.h>
#include <panda/types/DataTraits.h>

#include <QDialog>
#include <QScrollArea>
#include <QFormLayout>
#include <QWidget>
#include <QSpinBox>

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
	int rowTypeFullId;
	value_type valueCopy;
	QVector<DataWidgetPtr> dataWidgets;
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

	void readFromData(const value_type& v)
	{
		valueCopy = v;
		resize(row_trait::size(v));

		for(auto child : dataWidgets)
			child->updateWidgetValue();

		if(resizeSpinBox)
			resizeSpinBox->setValue(dataWidgets.size());
	}

	void writeToData(value_type& v)
	{
		for(auto child : dataWidgets)
			child->updateDataValue();

		v = valueCopy;
	}

	void resize(int nb)
	{
		int oldSize = dataWidgets.size();
		row_trait::resize(valueCopy, nb);

		// Update value pointers if they changed
		int updateNb = qMin(nb, oldSize);
		for(int i=0; i<updateNb; ++i)
			dataWidgets[i]->changeValuePointer(row_trait::get(valueCopy, i));

		if(oldSize == nb)
			return;

		if(oldSize > nb)
		{	// Removing
			dataWidgets.resize(nb);
			for(int i=oldSize-1; i>=nb; --i)
			{
				QLayoutItem* label = formLayout->itemAt(i, QFormLayout::LabelRole);
				QLayoutItem* field = formLayout->itemAt(i, QFormLayout::FieldRole);
				if(label && field)
				{
					label->widget()->hide();
					field->widget()->hide();
					formLayout->removeItem(label);
					formLayout->removeItem(field);
					delete label;
					delete field;
				}
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
				row_type* pValue = row_trait::get(valueCopy, i);
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
