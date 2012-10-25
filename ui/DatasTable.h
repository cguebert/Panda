#ifndef DATASTABLE_H
#define DATASTABLE_H

#include <QWidget>
#include <QStyledItemDelegate>

class QTableWidget;
class QLabel;
class QLineEdit;
class QPushButton;

namespace panda
{
	class PandaObject;
	class BaseData;
}

class DatasTable : public QWidget
{
	Q_OBJECT
public:
	explicit DatasTable(panda::PandaObject* document, QWidget *parent = 0);

protected:
	QTableWidget* tableWidget;
	QLabel* nameLabel;
	panda::PandaObject* document;
	
signals:
	
public slots:
	void populateTable(panda::PandaObject*);
};

class DataDelegate : public QStyledItemDelegate
{
	Q_OBJECT
public:
	DataDelegate(int dataColumn, QObject *parent = 0);
	virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	virtual QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
	virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
	virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;

private slots:
	void commitAndCloseEditor();

private:
	int dataColumn;
};

class DataItemWidget : public QWidget
{
	Q_OBJECT
public:
	DataItemWidget(panda::BaseData* data, QWidget *parent = 0);

	void setEditorData();
	QString setModelData();

signals:
	void editingFinished();

private slots:
	void onOpenEditDialog();

protected:
	panda::BaseData *parentData;
	bool dataIsSet;

	QLineEdit *lineEdit;
	QPushButton *pushButton;
};

#endif // DATASTABLE_H
