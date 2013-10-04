#ifndef DATASTABLE_H
#define DATASTABLE_H

#include <QWidget>
#include <QList>
#include <QSharedPointer>

#include <ui/DataWidget.h>

class QStackedLayout;
class QLabel;

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

	typedef QSharedPointer<BaseDataWidget> DataWidgetPtr;

protected:
	QStackedLayout* stackedLayout;
	QLabel* nameLabel;
	panda::PandaObject *document, *currentObject;

	QList<DataWidgetPtr> dataWidgets;

signals:

public slots:
	void populateTable(panda::PandaObject*);
	void onModifiedObject(panda::PandaObject*);
};

#endif // DATASTABLE_H
