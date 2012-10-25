#ifndef DATASEDITDIALOG_H
#define DATASEDITDIALOG_H

#include <QDialog>
#include <QStyledItemDelegate>

class QTableWidget;

namespace panda
{
    class PandaObject;
    class BaseData;
}

class DatasEditDialog : public QDialog
{
    Q_OBJECT
public:
    explicit DatasEditDialog(panda::BaseData* data, QWidget *parent = 0);

protected:
    QTableWidget* tableWidget;
    panda::BaseData* parentData;

    template <class T>
    T getItemValue(int row, int column);

signals:

public slots:
    void copyToData();
    void populateTable();
    void changeNbElements(int);
};

class DatasEditDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    DatasEditDelegate(bool animation, int type, QObject *parent = 0);
    virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

private slots:
    void commitAndCloseEditor();

private:
    bool animation; // If true, the first column is reserved for a double value
    int valueType;
};


#endif // DATASEDITDIALOG_H
