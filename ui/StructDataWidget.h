#ifndef STRUCTDATAWIDGET_H
#define STRUCTDATAWIDGET_H

#include <ui/DataWidget.h>

#include <QtWidgets>

template<class T>
class flat_data_trait
{
public:
	typedef T value_type;
	typedef T item_type;

	static size_t size() { return 1; }
	static QStringList header() { return QStringList(); }
	static const item_type get(const value_type& d, size_t /*i*/= 0) { return &d; }
	static void set(value_type& d, const item_type& v, size_t /*i*/= 0) { d = v; }
};

//***************************************************************//

template<>
class flat_data_trait<QPointF>
{
public:
	typedef QPointF value_type;
	typedef qreal item_type;

	static size_t size() { return 2; }
	static QStringList header()
	{
		QStringList header;
		header << "X" << "Y";
		return header;
	}
	static const item_type get(const value_type& d, size_t i = 0)
	{
		switch(i)
		{
		case 0: return d.x();
		case 1: return d.y();
		}

		return 0.;
	}
	static void set(value_type& d, const item_type& v, size_t i = 0)
	{
		switch(i)
		{
		case 0: return d.setX(v);
		case 1: return d.setY(v);
		}
	}
};

//***************************************************************//

template<>
class flat_data_trait<QRectF>
{
public:
	typedef QRectF value_type;
	typedef qreal item_type;

	static size_t size() { return 4; }
	static QStringList header()
	{
		QStringList header;
		header << "Left" << "Top" << "Right" << "Bottom";
		return header;
	}
	static const item_type get(const value_type& d, size_t i = 0)
	{
		switch(i)
		{
		case 0: return d.left();
		case 1: return d.top();
		case 2: return d.right();
		case 3: return d.bottom();
		}

		return 0.;
	}
	static void set(value_type& d, const item_type& v, size_t i = 0)
	{
		switch(i)
		{
		case 0: return d.setLeft(v);
		case 1: return d.setTop(v);
		case 2: return d.setRight(v);
		case 3: return d.setBottom(v);
		}
	}
};

//***************************************************************//

template<class T>
class vector_data_trait
{
public:
	typedef T vector_type;
	typedef T row_type;

	enum { is_vector = 0 };
	enum { is_single = 1 };
	static size_t size(const vector_type&) { return 1; }
	static const row_type* get(const vector_type& v, size_t i=0)
	{
		return !i ? &v : nullptr;
	}
	static void set(vector_type& v, const row_type& r, size_t i=0)
	{
		if(!i)
			v = r;
	}
	static void resize(vector_type& /*v*/, size_t /*s*/) {}
};

//***************************************************************//

template<class T>
class vector_data_trait< QVector<T> >
{
public:
	typedef QVector<T> vector_type;
	typedef T row_type;

	enum { is_vector = 1 };
	enum { is_single = 0 };
	static size_t size(const vector_type& v) { return v.size(); }
	static QStringList header(const vector_type&) { return QStringList{}; }
	static const row_type* get(const vector_type& v, size_t i=0)
	{
		return (i<size(v)) ? &(v[i]) : nullptr;
	}
	static void set(vector_type& v, const row_type& r, size_t i=0)
	{
		if(i < size(v))
			v[i] = r;
	}
	static void resize(vector_type& v, size_t s)
	{
		v.resize(s);
	}
};

//***************************************************************//

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
	typedef vector_data_trait<value_type> rowTrait;
	typedef typename rowTrait::row_type row_type;
	typedef flat_data_trait<row_type> itemTrait;
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

		QStringList headerLabels = itemTrait::header();
		tableWidget->setColumnCount(itemTrait::size());
		tableWidget->setHorizontalHeaderLabels(headerLabels);

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

		d.endEdit();
	}

	virtual void resizeValue()
	{
		int size = resizeSpinBox->value();
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
