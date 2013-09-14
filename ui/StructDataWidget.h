#ifndef STRUCTDATAWIDGET_H
#define STRUCTDATAWIDGET_H

#include <panda/DataTraits.h>
#include <ui/DataWidget.h>

#include <QtWidgets>


class BaseTableDataDialog : public QDialog
{
	Q_OBJECT
public:
	BaseTableDataDialog(QWidget* parent)
		: QDialog(parent)
	{}

signals:

public slots:
	virtual void resizeValue() {}
};

template<class T>
class TableDataDialog : public BaseTableDataDialog
{
protected:
	typedef T value_type;
	typedef panda::Data<T> data_type;
	typedef panda::vector_data_trait<value_type> rowTrait;
	typedef typename rowTrait::row_type row_type;
	typedef panda::flat_data_trait<row_type> itemTrait;
	typedef typename itemTrait::item_type item_type;

	bool readOnly;
	QTableWidget* tableWidget;
	QWidget* resizeWidget;
	QSpinBox* resizeSpinBox;

public:
	TableDataDialog(QWidget* parent, const data_type& d, bool readOnly)
		: BaseTableDataDialog(parent)
		, readOnly(readOnly)
		, tableWidget(nullptr)
		, resizeWidget(nullptr)
		, resizeSpinBox(nullptr)
	{
		QVBoxLayout* mainLayout = new QVBoxLayout(this);

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

		tableWidget = new QTableWidget(this);
		mainLayout->addWidget(tableWidget);

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

		setWindowTitle(d.getName() + (readOnly ? tr(" (read-only)") : ""));
	}

	void updateTable(const value_type& v)
	{
		Qt::ItemFlags itemFlags = Qt::ItemIsEnabled;
		if(!readOnly)
			itemFlags |= Qt::ItemIsEditable;

		size_t nbRows = rowTrait::size(v);
		size_t nbCols = itemTrait::size();


		tableWidget->setColumnCount(nbCols);
		if(nbCols > 1)
			tableWidget->setHorizontalHeaderLabels(itemTrait::header());
		else
			tableWidget->horizontalHeader()->hide();

		tableWidget->setRowCount(nbRows);

		for(size_t i = 0; i<nbRows; ++i)
		{
			const row_type* row = rowTrait::get(v, i);
			if(row)
			{
				for(size_t j=0; j<nbCols; ++j)
				{
					QString text;
					QTextStream stream(&text, QIODevice::WriteOnly);
					stream << itemTrait::get(*row, j);
					QTableWidgetItem *item = new QTableWidgetItem(text);
					item->setFlags(itemFlags);
					tableWidget->setItem(i, j, item);
				}
			}
		}
	}

	value_type readFromTable()
	{
		size_t nbRows = tableWidget->rowCount();
		size_t nbCols = itemTrait::size();

		value_type v;
		rowTrait::resize(v, nbRows);

		for(size_t i = 0; i<nbRows; ++i)
		{
			const row_type* row = rowTrait::get(v, i);
			if(row)
			{
				row_type rowVal = *row;
				for(size_t j=0; j<nbCols; ++j)
				{
					QTableWidgetItem *item = tableWidget->item(i, j);
					if(item)
					{
						item_type val;
						QString text = item->text();
						QTextStream stream(&text, QIODevice::ReadOnly);
						stream >> val;
						itemTrait::set(rowVal, val, j);
					}
				}

				rowTrait::set(v, rowVal, i);
			}
		}

		return v;
	}

	virtual void readFromData(const value_type& v)
	{
		updateTable(v);

		// Resize the view in single value mode
		if(rowTrait::is_single)
		{
			tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
			tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

			int w = tableWidget->verticalHeader()->width() + 20;
			for(size_t i=0, nb=tableWidget->columnCount(); i<nb; ++i)
				w += tableWidget->columnWidth(i);
			int h = tableWidget->horizontalHeader()->height() + 4 + tableWidget->rowHeight(0);

			tableWidget->setMinimumSize(w, h);
			tableWidget->setMaximumSize(w, h);

			layout()->setSizeConstraint(QLayout::SetFixedSize);
		}

		if(rowTrait::is_single || readOnly)
			resizeWidget->hide();
		else
			resizeSpinBox->setValue(tableWidget->rowCount());
	}

	virtual void writeToData(value_type& v)
	{
		v = readFromTable();
	}

	virtual void resizeValue()
	{
		int oldSize = tableWidget->rowCount();
		int size = resizeSpinBox->value();
		int nbCols = tableWidget->columnCount();

		tableWidget->setRowCount(size);

		if(oldSize < size)
		{
			itemTrait::item_type val = itemTrait::item_type();
			QString text;
			QTextStream stream(&text, QIODevice::WriteOnly);
			stream << val;

			Qt::ItemFlags itemFlags = Qt::ItemIsEnabled;
			if(!readOnly)
				itemFlags |= Qt::ItemIsEditable;

			for(int i=oldSize; i<size; ++i)
			{
				for(int j=0; j<nbCols; ++j)
				{
					QTableWidgetItem *item = new QTableWidgetItem(text);
					item->setFlags(itemFlags);
					tableWidget->setItem(i, j, item);
				}
			}
		}
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

template<class T, class Dialog = TableDataDialog<T> >
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
			MyTData* data = getData();
			value_type& v = *data->beginEdit();
			dialog->writeToData(v);
			data->endEdit();

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
		typedef panda::vector_data_trait<value_type> vector_trait;
		if(vector_trait::is_vector)
		{
			QString text = QString("<i>%1 elements</i>").arg(vector_trait::size(v));	// tr()
			label->setText(text);
		}
		else
			label->setText(panda::valueToString(v));
	}
};

#endif
