#ifndef TABLEDATAWIDGETDIALOG_H
#define TABLEDATAWIDGETDIALOG_H

#include <ui/widget/DataWidget.h>
#include <ui/widget/StructTraits.h>
#include <panda/types/DataTraits.h>

#include <QtWidgets>

// Table model for property widgets for lists of values
template <class T>
class TablePropertyModel : public QAbstractTableModel
{
public:
	using value_type = T;
	using row_trait = VectorDataTrait<value_type>;
	using row_type = typename row_trait::row_type;
	using item_trait = FlatDataTrait<row_type>;
	using item_type = typename item_trait::item_type;

	TablePropertyModel(QObject* parent, bool readOnly)
		: QAbstractTableModel(parent), m_readOnly(readOnly) { }

	const value_type& value() const { return m_value; }
	void setValue(const value_type& value) { beginResetModel(); m_value = value;  endResetModel(); }

	int rowCount() const { return row_trait::size(m_value); }
	int columnCount() const { return item_trait::size(); }
	bool isFixed() const { return row_trait::is_single; }

	int rowCount(const QModelIndex&) const override { return rowCount(); }
	int columnCount(const QModelIndex&) const override { return columnCount(); }

	QVariant data(const QModelIndex& index, int role) const
	{
		if (!index.isValid())
			return QVariant();

		if (role != Qt::DisplayRole && role != Qt::EditRole)
			return QVariant();

		return QVariant(item_trait::get(row_trait::get(m_value, index.row()), index.column()));
	}

	QVariant headerData(int section, Qt::Orientation orientation, int role) const
	{
		if(role != Qt::DisplayRole)
			return QVariant();

		if(orientation == Qt::Vertical)
			return QString::number(section);
		else
		{
			auto list = item_trait::header();
			if (list.empty()) return QString();
			return list[section];
		}
	}

	Qt::ItemFlags flags(const QModelIndex& index) const
	{ return m_readOnly ? 0 : Qt::ItemIsEditable | Qt::ItemIsEnabled; }

	bool setData(const QModelIndex& index, const QVariant& value, int role) override
	{
		if(role != Qt::EditRole)
			return false;

		item_trait::set(row_trait::get(m_value, index.row()), value.value<item_type>(), index.column());
		return true;
	}

	void resizeValue(int nb)
	{
		int prevNb = rowCount();
		if(nb == prevNb)
			return;

		if (nb > prevNb)
		{ beginInsertRows(QModelIndex(), prevNb, nb - 1); row_trait::resize(m_value, nb); endInsertRows(); }
		else
		{ beginRemoveRows(QModelIndex(), nb, prevNb - 1); row_trait::resize(m_value, nb); endRemoveRows(); }
	}

protected:
	bool m_readOnly;
	value_type m_value;
};

/*****************************************************************************/

template<class T>
class TableDataWidgetDialog : public QDialog
{
protected:
	using value_type = T;

	bool m_readOnly;
	QTableView* m_view = nullptr;
	TablePropertyModel<value_type>* m_model = nullptr;
	QSpinBox* m_resizeSpinBox = nullptr;
	QPushButton* m_toggleButton = nullptr;

public:
	TableDataWidgetDialog(QWidget* parent, bool readOnly, QString name)
		: QDialog(parent)
		, m_readOnly(readOnly)
	{
		m_view = new QTableView(parent);
		m_model = new TablePropertyModel<value_type>(m_view, m_readOnly);
		m_view->setModel(m_model);

		QVBoxLayout* mainLayout = new QVBoxLayout(this);

		if(m_model->isFixed())
		{
			// Hide headers
			m_view->verticalHeader()->hide();
			m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

			// Set height
			auto height = m_view->rowHeight(0) * m_model->rowCount() + m_view->horizontalHeader()->height() + 2;
			m_view->setMinimumHeight(height);
			m_view->setMaximumHeight(height);
		}
		else if(!m_readOnly)
		{
			auto resizeWidget = new QWidget(this);
			QLabel* resizeLabel = new QLabel(tr("Size"));
			m_resizeSpinBox = new QSpinBox();
			m_resizeSpinBox->setMinimum(0);
			m_resizeSpinBox->setMaximum(65535);
			QPushButton* resizeButton = new QPushButton(tr("Resize"));
			QHBoxLayout* resizeLayout = new QHBoxLayout;
			resizeLayout->setMargin(0);
			resizeLayout->addWidget(resizeLabel);
			resizeLayout->addWidget(m_resizeSpinBox, 1);
			resizeLayout->addWidget(resizeButton);
			resizeWidget->setLayout(resizeLayout);
			mainLayout->addWidget(resizeWidget);

			QObject::connect(resizeButton, &QPushButton::clicked, [this]() { resizeValue(); });
		}

		// Set min width
		int width = 0;
		for(int i = 0, nb = m_model->columnCount(); i < nb; ++i)
			width += m_view->columnWidth(i);
		m_view->setMinimumWidth(width + 25);

		if(m_model->columnCount() < 2)
			m_view->horizontalHeader()->hide();

		mainLayout->addWidget(m_view);

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

	virtual void readFromData(const value_type& v)
	{
		m_model->setValue(v);

		if(m_resizeSpinBox)
			m_resizeSpinBox->setValue(m_model->rowCount());
	}

	virtual void writeToData(value_type& v)
	{
		v = m_model->value();
	}

	void resizeValue()
	{
		int nb = m_resizeSpinBox->value();
		m_model->resizeValue(nb);
	}
};

#endif // TABLEDATAWIDGETDIALOG_H
