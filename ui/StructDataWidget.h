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
public:
	typedef T value_type;
	typedef panda::Data<value_type> data_type;
	typedef panda::vector_data_trait<value_type> rowTrait;
	typedef typename rowTrait::row_type row_type;
	typedef panda::flat_data_trait<row_type> itemTrait;
	typedef typename itemTrait::item_type item_type;

	bool readOnly;

	QTableWidget* tableWidget;
	QWidget* resizeWidget;
	QSpinBox* resizeSpinBox;

	value_type valueCopy;

	TableDataDialog(QWidget* parent)
		: BaseTableDataDialog(parent)
		, readOnly(true)
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
		value_type v;

		size_t nbRows = tableWidget->rowCount();
		size_t nbCols = itemTrait::size();

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

	virtual void readFromData(const data_type& d)
	{
		if(!d.isReadOnly() && !d.getParent())
			readOnly = false;
		else
			readOnly = true;

		setWindowTitle(d.getName() + (readOnly ? tr(" (read-only)") : ""));

		const value_type& v = d.getValue();
		valueCopy = v;

		updateTable(v);

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

	virtual void writeToData(data_type& d)
	{
		value_type& v = *d.beginEdit();
		v = readFromTable();
		d.endEdit();
	}

	virtual void resizeValue()
	{
		int size = resizeSpinBox->value();
		valueCopy = readFromTable();
		rowTrait::resize(valueCopy, size);
		updateTable(valueCopy);
	}
};

//***************************************************************//

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
	const data_type* data;

	open_dialog_data_widget_container()
		: dialog(nullptr)
		, container(nullptr)
		, data(nullptr)
	{}

	QWidget* createWidgets(BaseDataWidget* parent, const data_type& d)
	{
		data = &d;
		container = new QWidget(parent);
		QPushButton* pushButton = new QPushButton("...");

		QHBoxLayout* layout = new QHBoxLayout(container);
		layout->setMargin(0);
		layout->addWidget(pushButton);
		container->setLayout(layout);

		QObject::connect(pushButton, SIGNAL(clicked()), this, SLOT(onShowDialog()) );
		QObject::connect(this, SIGNAL(editingFinished()), parent, SLOT(setWidgetDirty()) );

		return container;
	}

	void readFromData(const data_type& d)
	{
		if(dialog)
			dialog->readFromData(d);
	}

	void writeToData(data_type& d)
	{
		if(dialog)
			dialog->writeToData(d);
	}

	void setWidgetEnabled(QWidget* /*widget*/, bool /*enable*/)
	{
		// TODO: when we have a preview of the data, grey the preview but keep the button
	}

	void onShowDialog()
	{
		if(!dialog)
			dialog = new Dialog(container);

		dialog->readFromData(*data);
		if(dialog->exec() == QDialog::Accepted)
			emit editingFinished();
	}
};

#endif
