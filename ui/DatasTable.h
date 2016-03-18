#ifndef DATASTABLE_H
#define DATASTABLE_H

#include <QWidget>

#include <memory>
#include <vector>

#include <ui/widget/DataWidget.h>
#include <panda/messaging.h>

class GraphView;
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
	explicit DatasTable(GraphView* view, QWidget* parent = nullptr);

	typedef std::shared_ptr<BaseDataWidget> DataWidgetPtr;

private:
	void updateCurrentObject();
	void queuePopulate(panda::PandaObject*);
	void onDirtyObject(panda::PandaObject*);
	void onModifiedObject(panda::PandaObject*);

	QStackedLayout* m_stackedLayout;
	QLabel* m_nameLabel;
	panda::PandaDocument* m_document;
	panda::PandaObject *m_currentObject, *m_nextObject;
	bool m_waitingPopulate;
	panda::msg::Observer m_observer;

	std::vector<DataWidgetPtr> m_dataWidgets;
	
public slots:
	void populateTable();
};

#endif // DATASTABLE_H
