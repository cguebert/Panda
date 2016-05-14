#ifndef DATASTABLE_H
#define DATASTABLE_H

#include <QWidget>

#include <memory>
#include <vector>

#include <ui/widget/DataWidget.h>
#include <panda/messaging.h>

class GraphView;
class ObjectWatcher;
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
	~DatasTable();

	void updateCurrentObject();
	void setSelectedObject(panda::PandaObject* object);

public slots:
	void populateTable();

private:
	void queuePopulate(panda::PandaObject*);
	void onModifiedObject(panda::PandaObject*);

	QStackedLayout* m_stackedLayout;
	QLabel* m_nameLabel;
	panda::PandaDocument* m_document;
	panda::PandaObject *m_currentObject = nullptr, *m_nextObject = nullptr;
	bool m_waitingPopulate = false;
	bool m_objectIsModified = false;
	panda::msg::Observer m_observer;

	using DataWidgetPtr = std::shared_ptr<BaseDataWidget>;
	std::vector<DataWidgetPtr> m_dataWidgets;

	std::unique_ptr<ObjectWatcher> m_objectWatcher;
};

#endif // DATASTABLE_H
