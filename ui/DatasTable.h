#ifndef DATASTABLE_H
#define DATASTABLE_H

#include <QWidget>

#include <memory>
#include <vector>

#include <ui/widget/DataWidget.h>
#include <panda/messaging.h>

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
	explicit DatasTable(panda::PandaDocument* m_document, QWidget* parent = nullptr);

	typedef std::shared_ptr<BaseDataWidget> DataWidgetPtr;

protected:
	QStackedLayout* m_stackedLayout;
	QLabel* m_nameLabel;
	panda::PandaDocument* m_document;
	panda::PandaObject *m_currentObject, *m_nextObject;
	bool m_waitingPopulate;
	panda::msg::Observer m_observer;

	std::vector<DataWidgetPtr> m_dataWidgets;
	
public slots:
	void queuePopulate(panda::PandaObject*);
	void populateTable();
	void onModifiedObject(panda::PandaObject*);
};

#endif // DATASTABLE_H
