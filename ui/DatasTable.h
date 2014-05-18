#ifndef DATASTABLE_H
#define DATASTABLE_H

#include <QWidget>
#include <QList>
#include <QSharedPointer>

#include <ui/widget/DataWidget.h>

class QStackedLayout;
class QLabel;

namespace panda
{
	class PandaDocument;
	class PandaObject;
	class BaseData;
}

class DatasTable : public QWidget
{
	Q_OBJECT
public:
	explicit DatasTable(panda::PandaDocument* m_document, QWidget *parent = 0);

	typedef QSharedPointer<BaseDataWidget> DataWidgetPtr;

protected:
	QStackedLayout* m_stackedLayout;
	QLabel* m_nameLabel;
	panda::PandaDocument* m_document;
	panda::PandaObject *m_currentObject, *m_nextObject;
	bool m_waitingPopulate;

	QList<DataWidgetPtr> m_dataWidgets;

signals:

public slots:
	void queuePopulate(panda::PandaObject*);
	void populateTable();
	void onModifiedObject(panda::PandaObject*);
};

#endif // DATASTABLE_H
