#include <QtGui>

#include <ui/DatasEditDialog.h>

#include <panda/PandaObject.h>
#include <panda/Animation.h>

typedef panda::Data<int> IntData;
typedef QVector<int> IntVector;
typedef panda::Data<IntVector> IntVectorData;

typedef panda::Data<double> DoubleData;
typedef QVector<double> DoubleVector;
typedef panda::Data<DoubleVector> DoubleVectorData;
typedef panda::Animation<double> DoubleAnimation;
typedef panda::Data<DoubleAnimation> DoubleAnimationData;

typedef panda::Data<QColor> ColorData;
typedef QVector<QColor> ColorVector;
typedef panda::Data<ColorVector> ColorVectorData;
typedef panda::Animation<QColor> ColorAnimation;
typedef panda::Data<ColorAnimation> ColorAnimationData;

typedef panda::Data<QPointF> PointData;
typedef QVector<QPointF> PointVector;
typedef panda::Data<PointVector> PointVectorData;
typedef panda::Animation<QPointF> PointAnimation;
typedef panda::Data<PointAnimation> PointAnimationData;

typedef panda::Data<QRectF> RectData;
typedef QVector<QRectF> RectVector;
typedef panda::Data<RectVector> RectVectorData;

typedef panda::Data<QString> StringData;
typedef QVector<QString> StringVector;
typedef panda::Data<StringVector> StringVectorData;

DatasEditDialog::DatasEditDialog(panda::BaseData* data, QWidget *parent)
    : QDialog(parent)
    , parentData(data)
{
    QVBoxLayout* vLayout = new QVBoxLayout;

    if(data->isVector() || data->isAnimation())
    {
        QLabel* nbElementsLabel = new QLabel(tr("Number of elements :"), this);
        QSpinBox* nbElementsSpinBox = new QSpinBox(this);
        nbElementsSpinBox->setValue(data->getSize());
        connect(nbElementsSpinBox, SIGNAL(valueChanged(int)), this, SLOT(changeNbElements(int)));

        QHBoxLayout* nbElementsLayout = new QHBoxLayout;
        nbElementsLayout->addWidget(nbElementsLabel);
        nbElementsLayout->addWidget(nbElementsSpinBox);
        vLayout->addLayout(nbElementsLayout);
    }

    tableWidget = new QTableWidget(this);
    tableWidget->setSelectionMode(QAbstractItemView::NoSelection);
    vLayout->addWidget(tableWidget);
    populateTable();
    tableWidget->setMinimumWidth(tableWidget->horizontalHeader()->length() + 10);

    tableWidget->setItemDelegate(new DatasEditDelegate(data->isAnimation(), data->getValueType(), this));

    QPushButton* okButton = new QPushButton(tr("Ok"), this);
    okButton->setDefault(true);
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
    QPushButton* cancelButton = new QPushButton(tr("Cancel"), this);
    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    QHBoxLayout* buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch();
    buttonsLayout->addWidget(okButton);
    buttonsLayout->addWidget(cancelButton);
    vLayout->addLayout(buttonsLayout);

    setLayout(vLayout);
}

void DatasEditDialog::populateTable()
{
    QStringList headerLabels;
    if(parentData->isAnimation())
        headerLabels << "time";

    tableWidget->setRowCount(parentData->getSize());

    Qt::ItemFlags itemFlags = Qt::ItemIsEnabled | Qt::ItemIsEditable;

    switch(parentData->getValueType())
    {
        case QMetaType::Int:
        {
            if(parentData->isSingleValue())
                headerLabels << "integer value";
            else
                headerLabels << "integer values";
            tableWidget->setColumnCount(headerLabels.count());
            tableWidget->setHorizontalHeaderLabels(headerLabels);

            if(parentData->isSingleValue())
            {
                IntData* intData = dynamic_cast<IntData*>(parentData);
                if(intData)
                {
                    QTableWidgetItem *item = new QTableWidgetItem(QString::number(intData->getValue()));
                    item->setData(Qt::UserRole, qVariantFromValue(intData->getValue()));
                    item->setFlags(itemFlags);
                    tableWidget->setItem(0,0,item);
                }
            }
            else if(parentData->isVector())
            {
                IntVectorData* vectorIntData = dynamic_cast<IntVectorData*>(parentData);
                if(vectorIntData)
                {
                    const IntVector& vectorInt = vectorIntData->getValue();
                    int nb = vectorInt.size();
                    for(int i=0; i<nb; ++i)
                    {
                        QTableWidgetItem *item = new QTableWidgetItem(QString::number(vectorInt[i]));
                        item->setData(Qt::UserRole, qVariantFromValue(vectorInt[i]));
                        item->setFlags(itemFlags);
                        tableWidget->setItem(i,0,item);
                    }
                }
            }

            break;
        }
        case QMetaType::Double:
        {
            if(parentData->isSingleValue())
                headerLabels << "real value";
            else
                headerLabels << "real values";
            tableWidget->setColumnCount(headerLabels.count());
            tableWidget->setHorizontalHeaderLabels(headerLabels);

            if(parentData->isSingleValue())
            {
                DoubleData* doubleData = dynamic_cast<DoubleData*>(parentData);
                if(doubleData)
                {
                    QTableWidgetItem *item = new QTableWidgetItem(QString::number(doubleData->getValue()));
                    item->setData(Qt::UserRole, qVariantFromValue(doubleData->getValue()));
                    item->setFlags(itemFlags);
                    tableWidget->setItem(0,0,item);
                }
            }
            else if(parentData->isVector())
            {
                DoubleVectorData* vectorDoubleData = dynamic_cast<DoubleVectorData*>(parentData);
                if(vectorDoubleData)
                {
                    const DoubleVector& vectorDouble = vectorDoubleData->getValue();
                    int nb = vectorDouble.size();
                    for(int i=0; i<nb; ++i)
                    {
                        QTableWidgetItem *item = new QTableWidgetItem(QString::number(vectorDouble[i]));
                        item->setData(Qt::UserRole, qVariantFromValue(vectorDouble[i]));
                        item->setFlags(itemFlags);
                        tableWidget->setItem(i,0,item);
                    }
                }
            }
            else if(parentData->isAnimation())
            {
                DoubleAnimationData* animDoubleData = dynamic_cast<DoubleAnimationData*>(parentData);
                if(animDoubleData)
                {
                    const DoubleAnimation& animDouble = animDoubleData->getValue();
                    DoubleAnimation::Iterator iter = animDouble.getIterator();
                    int i=0;
                    while(iter.hasNext())
                    {
                        iter.next();
                        QTableWidgetItem *item0 = new QTableWidgetItem(QString::number(iter.key()));
                        item0->setData(Qt::UserRole, qVariantFromValue(iter.key()));
                        item0->setFlags(itemFlags);
                        tableWidget->setItem(i,0,item0);

                        QTableWidgetItem *item1 = new QTableWidgetItem(QString::number(iter.value()));
                        item1->setData(Qt::UserRole, qVariantFromValue(iter.value()));
                        item1->setFlags(itemFlags);
                        tableWidget->setItem(i,1,item1);
                        ++i;
                    }
                }
            }
            break;
        }
        case QMetaType::QColor:
        {
            if(parentData->isSingleValue())
                headerLabels << "color value";
            else
                headerLabels << "color values";
            tableWidget->setColumnCount(headerLabels.count());
            tableWidget->setHorizontalHeaderLabels(headerLabels);

            if(parentData->isSingleValue())
            {
                ColorData* colorData = dynamic_cast<ColorData*>(parentData);
                if(colorData)
                {
                    QTableWidgetItem *item = new QTableWidgetItem(parentData->toString());
                    item->setData(Qt::UserRole, qVariantFromValue(colorData->getValue()));
                    item->setFlags(itemFlags);
                    tableWidget->setItem(0,0,item);
                }
            }
            else if(parentData->isVector())
            {
                ColorVectorData* vectorColorData = dynamic_cast<ColorVectorData*>(parentData);
                if(vectorColorData)
                {
                    const ColorVector& vectorColor = vectorColorData->getValue();
                    int nb = vectorColor.size();
                    for(int i=0; i<nb; ++i)
                    {
                        QTableWidgetItem *item = new QTableWidgetItem(QString("#%1").arg(vectorColor[i].rgba(), 8, 16, QChar('0')).toUpper());
                        item->setData(Qt::UserRole, qVariantFromValue(vectorColor[i]));
                        item->setFlags(itemFlags);
                        tableWidget->setItem(i,0,item);
                    }
                }
            }
            else if(parentData->isAnimation())
            {
                ColorAnimationData* animColorData = dynamic_cast<ColorAnimationData*>(parentData);
                if(animColorData)
                {
                    const ColorAnimation& animColor = animColorData->getValue();
                    ColorAnimation::Iterator iter = animColor.getIterator();
                    int i=0;
                    while(iter.hasNext())
                    {
                        iter.next();
                        QTableWidgetItem *item0 = new QTableWidgetItem(QString::number(iter.key()));
                        item0->setData(Qt::UserRole, qVariantFromValue(iter.key()));
                        item0->setFlags(itemFlags);
                        tableWidget->setItem(i,0,item0);

                        QTableWidgetItem *item1 = new QTableWidgetItem(QString("#%1").arg(iter.value().rgba(), 8, 16, QChar('0')).toUpper());
                        item1->setData(Qt::UserRole, qVariantFromValue(iter.value()));
                        item1->setFlags(itemFlags);
                        tableWidget->setItem(i,1,item1);
                        ++i;
                    }
                }
            }
            break;
        }
        case QMetaType::QPointF:
        {
            headerLabels << "x" << "y";
            tableWidget->setColumnCount(headerLabels.count());
            tableWidget->setHorizontalHeaderLabels(headerLabels);

            if(parentData->isSingleValue())
            {
                PointData* pointData = dynamic_cast<PointData*>(parentData);
                if(pointData)
                {
                    QPointF pt = pointData->getValue();
                    QTableWidgetItem *item0 = new QTableWidgetItem(QString::number(pt.x()));
                    item0->setData(Qt::UserRole, qVariantFromValue(pt.x()));
                    item0->setFlags(itemFlags);
                    tableWidget->setItem(0,0,item0);

                    QTableWidgetItem *item1 = new QTableWidgetItem(QString::number(pt.y()));
                    item1->setData(Qt::UserRole, qVariantFromValue(pt.y()));
                    item1->setFlags(itemFlags);
                    tableWidget->setItem(0,1,item1);
                }
            }
            else if(parentData->isVector())
            {
                PointVectorData* vectorPointData = dynamic_cast<PointVectorData*>(parentData);
                if(vectorPointData)
                {
                    const PointVector& vectorPoint = vectorPointData->getValue();
                    int nb = vectorPoint.size();
                    for(int i=0; i<nb; ++i)
                    {
                        QPointF pt = vectorPoint[i];
                        QTableWidgetItem *item0 = new QTableWidgetItem(QString::number(pt.x()));
                        item0->setData(Qt::UserRole, qVariantFromValue(pt.x()));
                        item0->setFlags(itemFlags);
                        tableWidget->setItem(i,0,item0);

                        QTableWidgetItem *item1 = new QTableWidgetItem(QString::number(pt.y()));
                        item1->setData(Qt::UserRole, qVariantFromValue(pt.y()));
                        item1->setFlags(itemFlags);
                        tableWidget->setItem(i,1,item1);
                    }
                }
            }
            else if(parentData->isAnimation())
            {
                PointAnimationData* animPointData = dynamic_cast<PointAnimationData*>(parentData);
                if(animPointData)
                {
                    const PointAnimation& animPoint = animPointData->getValue();
                    PointAnimation::Iterator iter = animPoint.getIterator();
                    int i=0;
                    while(iter.hasNext())
                    {
                        iter.next();
                        QTableWidgetItem *item0 = new QTableWidgetItem(QString::number(iter.key()));
                        item0->setData(Qt::UserRole, qVariantFromValue(iter.key()));
                        item0->setFlags(itemFlags);
                        tableWidget->setItem(i,0,item0);

                        QPointF pt = iter.value();
                        QTableWidgetItem *item1 = new QTableWidgetItem(QString::number(pt.x()));
                        item1->setData(Qt::UserRole, qVariantFromValue(pt.x()));
                        item1->setFlags(itemFlags);
                        tableWidget->setItem(i,1,item1);

                        QTableWidgetItem *item2 = new QTableWidgetItem(QString::number(pt.y()));
                        item2->setData(Qt::UserRole, qVariantFromValue(pt.y()));
                        item2->setFlags(itemFlags);
                        tableWidget->setItem(i,2,item2);
                        ++i;
                    }
                }
            }
            break;
        }
        case QMetaType::QRectF:
        {
            headerLabels << "left" << "top" << "right" << "bottom";
            tableWidget->setColumnCount(headerLabels.count());
            tableWidget->setHorizontalHeaderLabels(headerLabels);

            if(parentData->isSingleValue())
            {
                RectData* rectData = dynamic_cast<RectData*>(parentData);
                if(rectData)
                {
                    QRectF r = rectData->getValue();
                    QTableWidgetItem *item0 = new QTableWidgetItem(QString::number(r.left()));
                    item0->setData(Qt::UserRole, qVariantFromValue(r.left()));
                    item0->setFlags(itemFlags);
                    tableWidget->setItem(0,0,item0);

                    QTableWidgetItem *item1 = new QTableWidgetItem(QString::number(r.top()));
                    item1->setData(Qt::UserRole, qVariantFromValue(r.top()));
                    item1->setFlags(itemFlags);
                    tableWidget->setItem(0,1,item1);

                    QTableWidgetItem *item2 = new QTableWidgetItem(QString::number(r.right()));
                    item2->setData(Qt::UserRole, qVariantFromValue(r.right()));
                    item2->setFlags(itemFlags);
                    tableWidget->setItem(0,2,item2);

                    QTableWidgetItem *item3 = new QTableWidgetItem(QString::number(r.bottom()));
                    item2->setData(Qt::UserRole, qVariantFromValue(r.bottom()));
                    item3->setFlags(itemFlags);
                    tableWidget->setItem(0,3,item3);
                }
            }
            else if(parentData->isVector())
            {
                RectVectorData* vectorRectData = dynamic_cast<RectVectorData*>(parentData);
                if(vectorRectData)
                {
                    const RectVector& vectorRect = vectorRectData->getValue();
                    int nb = vectorRect.size();
                    for(int i=0; i<nb; ++i)
                    {
                        QRectF r = vectorRect[i];
                        QTableWidgetItem *item0 = new QTableWidgetItem(QString::number(r.left()));
                        item0->setData(Qt::UserRole, qVariantFromValue(r.left()));
                        item0->setFlags(itemFlags);
                        tableWidget->setItem(i,0,item0);

                        QTableWidgetItem *item1 = new QTableWidgetItem(QString::number(r.top()));
                        item1->setData(Qt::UserRole, qVariantFromValue(r.top()));
                        item1->setFlags(itemFlags);
                        tableWidget->setItem(i,1,item1);

                        QTableWidgetItem *item2 = new QTableWidgetItem(QString::number(r.right()));
                        item2->setData(Qt::UserRole, qVariantFromValue(r.right()));
                        item2->setFlags(itemFlags);
                        tableWidget->setItem(i,2,item2);

                        QTableWidgetItem *item3 = new QTableWidgetItem(QString::number(r.bottom()));
                        item3->setData(Qt::UserRole, qVariantFromValue(r.bottom()));
                        item3->setFlags(itemFlags);
                        tableWidget->setItem(i,3,item3);
                    }
                }
            }
            break;
        }
        case QMetaType::QString:
        {
            if(parentData->isSingleValue())
                headerLabels << "text value";
            else
                headerLabels << "text values";
            tableWidget->setColumnCount(headerLabels.count());
            tableWidget->setHorizontalHeaderLabels(headerLabels);

            if(parentData->isSingleValue())
            {
                StringData* stringData = dynamic_cast<StringData*>(parentData);
                if(stringData)
                {
                    QTableWidgetItem *item = new QTableWidgetItem(stringData->getValue());
                    item->setData(Qt::UserRole, qVariantFromValue(stringData->getValue()));
                    item->setFlags(itemFlags);
                    tableWidget->setItem(0,0,item);
                }
            }
            else if(parentData->isVector())
            {
                StringVectorData* vectorStringData = dynamic_cast<StringVectorData*>(parentData);
                if(vectorStringData)
                {
                    const StringVector& vectorString = vectorStringData->getValue();
                    int nb = vectorString.size();
                    for(int i=0; i<nb; ++i)
                    {
                        QTableWidgetItem *item = new QTableWidgetItem(vectorString[i]);
                        item->setData(Qt::UserRole, qVariantFromValue(vectorString[i]));
                        item->setFlags(itemFlags);
                        tableWidget->setItem(i,0,item);
                    }
                }
            }
            break;
        }
    }

    if(parentData->isSingleValue())
    {
        tableWidget->verticalHeader()->setEnabled(false);
        tableWidget->verticalHeader()->hide();
    }
}

template <class T>
T DatasEditDialog::getItemValue(int row, int column)
{
    QTableWidgetItem* item = tableWidget->item(row, column);
    if(item)
        return item->data(Qt::UserRole).value<T>();
    else
        return T();
}

void DatasEditDialog::copyToData()
{
    switch(parentData->getValueType())
    {
    case QMetaType::Int:
        if(parentData->isSingleValue())
        {
            IntData* intData = dynamic_cast<IntData*>(parentData);
            if(intData)
                intData->setValue(getItemValue<int>(0,0));
        }
        else if(parentData->isVector())
        {
            IntVectorData* vectorIntData = dynamic_cast<IntVectorData*>(parentData);
            if(vectorIntData)
            {
                IntVector& vectorInt = *vectorIntData->beginEdit();
                int nb = tableWidget->rowCount();
                vectorInt.resize(nb);
                for(int i=0; i<nb; ++i)
                    vectorInt[i] = getItemValue<int>(i,0);
                vectorIntData->endEdit();
            }
        }
        break;
    case QMetaType::Double:
        if(parentData->isSingleValue())
        {
            DoubleData* doubleData = dynamic_cast<DoubleData*>(parentData);
            if(doubleData)
                doubleData->setValue(getItemValue<double>(0,0));
        }
        else if(parentData->isVector())
        {
            DoubleVectorData* vectorDoubleData = dynamic_cast<DoubleVectorData*>(parentData);
            if(vectorDoubleData)
            {
                DoubleVector& vectorDouble = *vectorDoubleData->beginEdit();
                int nb = tableWidget->rowCount();
                vectorDouble.resize(nb);
                for(int i=0; i<nb; ++i)
                    vectorDouble[i] = getItemValue<double>(i,0);
                vectorDoubleData->endEdit();
            }
        }
        else if(parentData->isAnimation())
        {
            DoubleAnimationData* animDoubleData = dynamic_cast<DoubleAnimationData*>(parentData);
            if(animDoubleData)
            {
                DoubleAnimation& animDouble = *animDoubleData->beginEdit();
                int nb = tableWidget->rowCount();
                animDouble.clear();
                for(int i=0; i<nb; ++i)
                {
                    double key = getItemValue<double>(i,0);
                    double val = getItemValue<double>(i,1);
                    animDouble.add(key, val);
                }
                animDoubleData->endEdit();
            }
        }
        break;
    case QMetaType::QColor:
        if(parentData->isSingleValue())
        {
            ColorData* colorData = dynamic_cast<ColorData*>(parentData);
            if(colorData)
                colorData->setValue(getItemValue<QColor>(0,0));
        }
        else if(parentData->isVector())
        {
            ColorVectorData* vectorColorData = dynamic_cast<ColorVectorData*>(parentData);
            if(vectorColorData)
            {
                ColorVector& vectorColor = *vectorColorData->beginEdit();
                int nb = tableWidget->rowCount();
                vectorColor.resize(nb);
                for(int i=0; i<nb; ++i)
                    vectorColor[i] = getItemValue<QColor>(i,0);
                vectorColorData->endEdit();
            }
        }
        else if(parentData->isAnimation())
        {
            ColorAnimationData* animColorData = dynamic_cast<ColorAnimationData*>(parentData);
            if(animColorData)
            {
                ColorAnimation& animColor = *animColorData->beginEdit();
                int nb = tableWidget->rowCount();
                animColor.clear();
                for(int i=0; i<nb; ++i)
                {
                    double key = getItemValue<double>(i,0);
                    QColor val = getItemValue<QColor>(i,1);
                    animColor.add(key, val);
                }
                animColorData->endEdit();
            }
        }
        break;
    case QMetaType::QPointF:
        if(parentData->isSingleValue())
        {
            PointData* pointData = dynamic_cast<PointData*>(parentData);
            if(pointData)
                pointData->setValue(QPointF(getItemValue<double>(0,1), getItemValue<double>(0,1)));
        }
        else if(parentData->isVector())
        {
            PointVectorData* vectorPointData = dynamic_cast<PointVectorData*>(parentData);
            if(vectorPointData)
            {
                PointVector& vectorPoint = *vectorPointData->beginEdit();
                int nb = tableWidget->rowCount();
                vectorPoint.resize(nb);
                for(int i=0; i<nb; ++i)
                    vectorPoint[i] = QPointF(getItemValue<double>(i,0), getItemValue<double>(i,1));
                vectorPointData->endEdit();
            }
        }
        else if(parentData->isAnimation())
        {
            PointAnimationData* animPointData = dynamic_cast<PointAnimationData*>(parentData);
            if(animPointData)
            {
                PointAnimation& animPoint = *animPointData->beginEdit();
                int nb = tableWidget->rowCount();
                animPoint.clear();
                for(int i=0; i<nb; ++i)
                {
                    double key = getItemValue<double>(i,0);
                    QPointF val = QPointF(getItemValue<double>(i,1), getItemValue<double>(i,2));
                    animPoint.add(key, val);
                }
                animPointData->endEdit();
            }
        }
        break;
    case QMetaType::QRectF:
        if(parentData->isSingleValue())
        {
            RectData* rectData = dynamic_cast<RectData*>(parentData);
            if(rectData)
            {
                double l, t, r, b;
                l = getItemValue<double>(0,0);
                t = getItemValue<double>(0,1);
                r = getItemValue<double>(0,2);
                b = getItemValue<double>(0,3);
                rectData->setValue(QRectF(l, t, r-l, b-t));
            }
        }
        else if(parentData->isVector())
        {
            RectVectorData* vectorRectData = dynamic_cast<RectVectorData*>(parentData);
            if(vectorRectData)
            {
                RectVector& vectorRect = *vectorRectData->beginEdit();
                int nb = tableWidget->rowCount();
                vectorRect.resize(nb);
                for(int i=0; i<nb; ++i)
                {
                    double l, t, r, b;
                    l = getItemValue<double>(i,0);
                    t = getItemValue<double>(i,1);
                    r = getItemValue<double>(i,2);
                    b = getItemValue<double>(i,3);
                    vectorRect[i] = QRectF(l, t, r-l, b-t);
                }
                vectorRectData->endEdit();
            }
        }
        break;
    case QMetaType::QString:
        if(parentData->isSingleValue())
        {
            StringData* stringData = dynamic_cast<StringData*>(parentData);
            if(stringData)
                stringData->setValue(getItemValue<QString>(0,0));
        }
        else if(parentData->isVector())
        {
            StringVectorData* vectorStringData = dynamic_cast<StringVectorData*>(parentData);
            if(vectorStringData)
            {
                StringVector& vectorString = *vectorStringData->beginEdit();
                int nb = tableWidget->rowCount();
                vectorString.resize(nb);
                for(int i=0; i<nb; ++i)
                    vectorString[i] = getItemValue<QString>(i,0);
                vectorStringData->endEdit();
            }
        }
        break;
    }
}


void DatasEditDialog::changeNbElements(int nb)
{
    tableWidget->setRowCount(nb);
}

//***************************************************************//

DatasEditDelegate::DatasEditDelegate(bool animation, int type, QObject *parent)
    : QStyledItemDelegate(parent)
    , animation(animation)
    , valueType(type)
{
}

QWidget* DatasEditDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem& /*option*/, const QModelIndex &index) const
{
    QLineEdit* editor = new QLineEdit(parent);

    if(animation && index.column() == 0)
        editor->setValidator(new QDoubleValidator());
    else
    {
        switch(valueType)
        {
        case QMetaType::Int:
            editor->setValidator(new QIntValidator());
            break;
        case QMetaType::Double:
        case QMetaType::QRectF:
        case QMetaType::QPointF:
            editor->setValidator(new QDoubleValidator());
            break;
        case QMetaType::QColor:
            editor->setValidator(new QRegExpValidator(QRegExp("(\\#?[0-9A-Fa-f]{6,8}(\\ \\#?[0-9A-Fa-f]{6,8})*\\ ?)*")));
            break;
        }
    }

    connect(editor, SIGNAL(editingFinished()), this, SLOT(commitAndCloseEditor()));
    return editor;
}

void DatasEditDelegate::commitAndCloseEditor()
{
    QWidget *editor = qobject_cast<QWidget *>(sender());
    emit commitData(editor);
    emit closeEditor(editor);
}

void DatasEditDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QLineEdit* lineEditor = qobject_cast<QLineEdit*>(editor);
    if(lineEditor)
        lineEditor->setText(index.data().toString());
    else
        QStyledItemDelegate::setEditorData(editor, index);
}

void DatasEditDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    QLineEdit* lineEditor = qobject_cast<QLineEdit*>(editor);
    if(lineEditor)
    {
        QString editorText = lineEditor->text();
        model->setData(index, qVariantFromValue(editorText));

        switch(valueType)
        {
        case QMetaType::Int:
            model->setData(index, qVariantFromValue(editorText.toInt()), Qt::UserRole);
            break;
        case QMetaType::Double:
        case QMetaType::QRectF:
        case QMetaType::QPointF:
            model->setData(index, qVariantFromValue(editorText.toDouble()), Qt::UserRole);
            break;
        case QMetaType::QColor:
            {
                if(editorText.startsWith('#'))
                    editorText = editorText.mid(1);
                QColor col;
                col.setRgba(editorText.toUInt(NULL, 16));
                model->setData(index, qVariantFromValue(col), Qt::UserRole);
            }
            break;
        case QMetaType::QString:
            model->setData(index, qVariantFromValue(editorText), Qt::UserRole);
            break;
        }
    }
    QStyledItemDelegate::setModelData(editor, model, index);
}
