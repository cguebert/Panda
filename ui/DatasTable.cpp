#include <QtGui>

#include <ui/DatasTable.h>
#include <ui/DatasEditDialog.h>

#include <panda/PandaObject.h>

DatasTable::DatasTable(panda::PandaObject* doc, QWidget *parent)
	: QWidget(parent)
	, document(doc)
{
	tableWidget = new QTableWidget(this);
	tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
	tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
	tableWidget->setColumnCount(2);
	QStringList headerLabels;
	headerLabels << "Property" << "Value";
	tableWidget->setHorizontalHeaderLabels(headerLabels);
	tableWidget->verticalHeader()->setEnabled(false);
	tableWidget->verticalHeader()->hide();

	tableWidget->setItemDelegate(new DataDelegate(1, this));

	tableWidget->horizontalHeader()->setStretchLastSection(true);

	nameLabel = new QLabel("Document");

	QVBoxLayout* mainLayout = new QVBoxLayout();
	mainLayout->addWidget(nameLabel);
	mainLayout->addWidget(tableWidget);
	setLayout(mainLayout);

	populateTable((panda::PandaObject*)doc);

	connect(doc, SIGNAL(selectedObject(panda::PandaObject*)), this, SLOT(populateTable(panda::PandaObject*)));
	connect(doc, SIGNAL(selectedObjectIsDirty(panda::PandaObject*)), this, SLOT(populateTable(panda::PandaObject*)));
}

void DatasTable::populateTable(panda::PandaObject* object)
{
	tableWidget->clearContents();
	tableWidget->setRowCount(0);

	if(!object)
		object = document;

	nameLabel->setText(object->getName());

	int rowIndex = 0;
	// inputs (or editable)
	foreach(panda::BaseData* data, object->getDatas())
	{
		if(!data->isDisplayed() || data->isReadOnly())
			continue;

		tableWidget->setRowCount(rowIndex+1);
		QTableWidgetItem *item0 = new QTableWidgetItem(data->getName());
		item0->setFlags(Qt::ItemIsEnabled);
		QTableWidgetItem *item1 = new QTableWidgetItem(data->toString());
		if(data->getParent())
			item1->setFlags(Qt::NoItemFlags);
		else
			item1->setFlags(Qt::ItemIsEnabled | Qt::ItemIsEditable);
		item1->setData(Qt::UserRole, QVariant::fromValue((void*)data));
		tableWidget->setItem(rowIndex, 0, item0);
		tableWidget->setItem(rowIndex, 1, item1);
		++rowIndex;
	}

	// outputs (or read only)
	foreach(panda::BaseData* data, object->getDatas())
	{
		if(!data->isDisplayed() || !data->isReadOnly())
			continue;

		tableWidget->setRowCount(rowIndex+1);
		QTableWidgetItem *item0 = new QTableWidgetItem(data->getName());
		item0->setFlags(Qt::NoItemFlags);
		QTableWidgetItem *item1 = new QTableWidgetItem(data->toString());
		item1->setFlags(Qt::NoItemFlags);
		tableWidget->setItem(rowIndex, 0, item0);
		tableWidget->setItem(rowIndex, 1, item1);
		++rowIndex;
	}
}

//***************************************************************//

DataDelegate::DataDelegate(int dataColumn, QObject *parent)
	: QStyledItemDelegate(parent)
	, dataColumn(dataColumn)
{
}

void DataDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	panda::BaseData* data = (panda::BaseData*)index.data(Qt::UserRole).value<void*>();
	if(data)
	{
		if(data->isSingleValue())
		{
			switch(data->getValueType())
			{
				case QMetaType::QColor:
				{
					panda::Data<QColor>* colData = dynamic_cast<panda::Data<QColor>*>(data);
					if(colData)
					{
						painter->fillRect(option.rect, QBrush(colData->getValue()));
						return;
					}
				}
			}
		}
	}

	QStyledItemDelegate::paint(painter, option, index);
}

void DataDelegate::commitAndCloseEditor()
{
	QWidget *editor = qobject_cast<QWidget *>(sender());
	emit commitData(editor);
	emit closeEditor(editor);
}

QWidget* DataDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	panda::BaseData* data = (panda::BaseData*)index.data(Qt::UserRole).value<void*>();
	if(data)
	{
		DataItemWidget* editor = new DataItemWidget(data, parent);
		connect(editor, SIGNAL(editingFinished()), this, SLOT(commitAndCloseEditor()));
		return editor;
	}
	else
		return QStyledItemDelegate::createEditor(parent, option, index);
}

void DataDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	panda::BaseData* data = (panda::BaseData*)index.data(Qt::UserRole).value<void*>();
	if(data)
	{
		DataItemWidget* itemEditor = qobject_cast<DataItemWidget *>(editor);
		if(itemEditor)
			itemEditor->setEditorData();
	}
	else
		QStyledItemDelegate::setEditorData(editor, index);
}

void DataDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	panda::BaseData* data = (panda::BaseData*)index.data(Qt::UserRole).value<void*>();
	if(data)
	{
		DataItemWidget* itemEditor = qobject_cast<DataItemWidget *>(editor);
		if(itemEditor)
		{
			QString text = itemEditor->setModelData();
			model->setData(index, qVariantFromValue(text));
		}
	}
	else
		QStyledItemDelegate::setModelData(editor, model, index);
}

//***************************************************************//

DataItemWidget::DataItemWidget(panda::BaseData *data, QWidget *parent)
	: QWidget(parent)
	, parentData(data)
	, dataIsSet(false)
{
	setBackgroundRole(QPalette::Window);
	lineEdit = new QLineEdit(this);
	lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	pushButton = new QPushButton("...", this);
	pushButton->setMaximumWidth(20);
	pushButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	pushButton->hide();
	connect(pushButton, SIGNAL(clicked()), this, SLOT(onOpenEditDialog()));

	QHBoxLayout* layout = new QHBoxLayout();
	layout->addWidget(lineEdit);
	layout->addWidget(pushButton);
	layout->setContentsMargins(0,0,0,0);
	setLayout(layout);

	setFocusProxy(lineEdit);
	setAutoFillBackground(true);
}

void DataItemWidget::setEditorData()
{
	lineEdit->setText(parentData->toString());

	if(parentData->isSingleValue())
	{
		switch(parentData->getValueType())
		{
		case QMetaType::Int:
			lineEdit->setValidator(new QIntValidator());
			break;
		case QMetaType::Double:
			lineEdit->setValidator(new QDoubleValidator());
			break;
		case QMetaType::QColor:
			lineEdit->setValidator(new QRegExpValidator(QRegExp("\\#?[0-9A-Fa-f]{6,8}")));
			pushButton->show();
			break;
		case QMetaType::QPointF:
			lineEdit->setValidator(new QRegExpValidator(QRegExp("\\-?\\d*\\.?\\d+ \\-?\\d*\\.?\\d+")));
			break;
		case QMetaType::QRectF:
			lineEdit->setValidator(new QRegExpValidator(QRegExp("\\-?\\d*\\.?\\d+ \\-?\\d*\\.?\\d+ \\-?\\d*\\.?\\d+ \\-?\\d*\\.?\\d+")));
			break;
		}
	}
	else if(parentData->isVector())
	{
		pushButton->show();
		switch(parentData->getValueType())
		{
		case QMetaType::Int:
			lineEdit->setValidator(new QRegExpValidator(QRegExp("(\\-?\\d+\\ ?)*")));
			break;
		case QMetaType::Double:
			lineEdit->setValidator(new QRegExpValidator(QRegExp("(\\-?\\d*\\.?\\d+\\ ?)*")));
			break;
		case QMetaType::QColor:
			lineEdit->setValidator(new QRegExpValidator(QRegExp("(\\#?[0-9A-Fa-f]{6,8}(\\ \\#?[0-9A-Fa-f]{6,8})*\\ ?)*")));
			break;
		case QMetaType::QPointF:
			lineEdit->setValidator(new QRegExpValidator(QRegExp("(\\-?\\d+\\.?\\d+\\ \\-?\\d+\\.?\\d+\\ ?)*")));
			break;
		case QMetaType::QRectF:
			lineEdit->setValidator(new QRegExpValidator(QRegExp("(\\-?\\d+\\.?\\d+ \\-?\\d+\\.?\\d+ \\-?\\d+\\.?\\d+ \\-?\\d+\\.?\\d+\\ ?)*")));
			break;
		}
	}
	else if(parentData->isAnimation())
	{
		pushButton->show();
		switch(parentData->getValueType())
		{
		case QMetaType::Double:
			lineEdit->setValidator(new QRegExpValidator(QRegExp("(\\d*\\.?\\d+\\ \\-?\\d*\\.?\\d+\\ ?)*")));
			break;
		case QMetaType::QColor:
			lineEdit->setValidator(new QRegExpValidator(QRegExp("(\\d*\\.?\\d+\\ \\#?[0-9A-Fa-f]{6,8}\\ ?)*")));
			break;
		case QMetaType::QPointF:
			lineEdit->setValidator(new QRegExpValidator(QRegExp("(\\d*\\.?\\d+\\ \\-?\\d+\\.?\\d+\\ \\-?\\d+\\.?\\d+\\ ?)*")));
			break;
		}
	}
}

QString DataItemWidget::setModelData()
{
	QString text = lineEdit->text();
	if(!dataIsSet)
		parentData->fromString(text);
	return text;
}

void DataItemWidget::onOpenEditDialog()
{
	if(parentData->isVector() || parentData->isAnimation())
	{
		DatasEditDialog dialog(parentData, this);
		if(dialog.exec())
		{
			dataIsSet = true;
			dialog.copyToData();
			lineEdit->setText(parentData->toString());
			emit editingFinished();
		}
	}
	else if(parentData->isSingleValue())
	{
		switch(parentData->getValueType())
		{
			case QMetaType::QColor:
			{
				panda::Data<QColor>* colorData = dynamic_cast<panda::Data<QColor>*>(parentData);
				if(colorData)
				{
					QColor color = QColorDialog::getColor(colorData->getValue(), this, colorData->getName(), QColorDialog::ShowAlphaChannel);
					if(color.isValid())
					{
						dataIsSet = true;
						colorData->setValue(color);
						lineEdit->setText(QString("#%1").arg(color.rgba(), 8, 16, QChar('0')).toUpper());
						emit editingFinished();
					}
				}
				break;
			}
		}
	}
}
