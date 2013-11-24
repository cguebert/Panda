#ifndef TABLEDATAWIDGETDIALOG_H
#define TABLEDATAWIDGETDIALOG_H

#include <ui/DataWidget.h>
#include <ui/StructTraits.h>
#include <panda/types/DataTraits.h>

#include <QtWidgets>

class BaseTableDataWidgetDialog : public QDialog
{
	Q_OBJECT
public:
	BaseTableDataWidgetDialog(QWidget* parent)
		: QDialog(parent)
	{}

signals:

public slots:
	virtual void resizeValue() {}
};

template<class T>
class TableDataWidgetDialog : public BaseTableDataWidgetDialog
{
protected:
	typedef T value_type;
	typedef panda::Data<value_type> data_type;
	typedef VectorDataTrait<value_type> row_trait;
	typedef typename row_trait::row_type row_type;
	typedef FlatDataTrait<row_type> item_trait;
	typedef typename item_trait::item_type item_type;

	bool readOnly;
	QTableWidget* tableWidget;
	QWidget* resizeWidget;
	QSpinBox* resizeSpinBox;

public:
	TableDataWidgetDialog(QWidget* parent, bool readOnly, QString name)
		: BaseTableDataWidgetDialog(parent)
		, readOnly(readOnly)
		, tableWidget(nullptr)
		, resizeWidget(nullptr)
		, resizeSpinBox(nullptr)
	{
		QVBoxLayout* mainLayout = new QVBoxLayout(this);

		if(!readOnly && row_trait::is_vector)
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
		setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

		setWindowTitle(name + (readOnly ? tr(" (read-only)") : ""));
	}

	void updateTable(const value_type& v)
	{
		Qt::ItemFlags itemFlags = Qt::ItemIsEnabled;
		if(!readOnly)
			itemFlags |= Qt::ItemIsEditable;

		int nbRows = row_trait::size(v);
		int nbCols = item_trait::size();

		tableWidget->setColumnCount(nbCols);
		if(nbCols > 1)
			tableWidget->setHorizontalHeaderLabels(item_trait::header());
		else
			tableWidget->horizontalHeader()->hide();

		tableWidget->setRowCount(nbRows);

		for(int i = 0; i<nbRows; ++i)
		{
			const row_type* row = row_trait::get(v, i);
			if(row)
			{
				for(int j=0; j<nbCols; ++j)
				{
					QString text = QString("%1").arg(item_trait::get(*row, j));
					QTableWidgetItem *item = new QTableWidgetItem(text);
					item->setFlags(itemFlags);
					tableWidget->setItem(i, j, item);
				}
			}
		}
	}

	value_type readFromTable()
	{
		int nbRows = tableWidget->rowCount();
		int nbCols = item_trait::size();

		value_type v;
		row_trait::resize(v, nbRows);

		for(int i = 0; i<nbRows; ++i)
		{
			const row_type* row = row_trait::get(v, i);
			if(row)
			{
				row_type rowVal = *row;
				for(int j=0; j<nbCols; ++j)
				{
					QTableWidgetItem *item = tableWidget->item(i, j);
					if(item)
					{
						QString text = item->text();
						QTextStream stream(&text);
						item_type val;
						stream >> val;
						item_trait::set(rowVal, val, j);
					}
				}

				row_trait::set(v, rowVal, i);
			}
		}

		return v;
	}

	virtual void readFromData(const value_type& v)
	{
		updateTable(v);

		// Resize the view in single value mode
		if(row_trait::is_single)
		{
			tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
			tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

			int w = tableWidget->verticalHeader()->width() + 20;
			for(int i=0, nb=tableWidget->columnCount(); i<nb; ++i)
				w += tableWidget->columnWidth(i);
			int h = tableWidget->horizontalHeader()->height() + 4 + tableWidget->rowHeight(0);

			tableWidget->setMinimumSize(w, h);
			tableWidget->setMaximumSize(w, h);

			layout()->setSizeConstraint(QLayout::SetFixedSize);
		}

		if(resizeSpinBox)
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
			item_trait::item_type val = item_trait::item_type();
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

#endif // TABLEDATAWIDGETDIALOG_H
