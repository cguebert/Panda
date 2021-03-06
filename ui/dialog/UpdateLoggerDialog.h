#ifndef UPDATELOGGERDIALOG_H
#define UPDATELOGGERDIALOG_H

#include <panda/helper/UpdateLogger.h>

#include <QStylePainter>
#include <QtWidgets>

class UpdateLoggerView;

class UpdateLoggerDialog : public QDialog
{
	Q_OBJECT
public:
	explicit UpdateLoggerDialog(QWidget* parent = nullptr);
	void updateEvents();

	static UpdateLoggerDialog* getInstance();
	static void setInstance(UpdateLoggerDialog* dlg);

	const panda::helper::EventData* getSelectedEvent() const;
	const panda::helper::UpdateLogger::NodeStates& getNodeStates() const;

protected:
	UpdateLoggerView* m_view;
	QLabel* m_label;
	static UpdateLoggerDialog* m_instance;

signals:
	void changedSelectedEvent();

public slots:
	void setEventText(QString);
};

//****************************************************************************//

class UpdateLoggerView : public QWidget
{
	Q_OBJECT
public:
	explicit UpdateLoggerView(QWidget* parent = nullptr);

	QSize minimumSizeHint() const;
	QSize sizeHint() const;

	const panda::helper::EventData* getSelectedEvent() const;
	const panda::helper::UpdateLogger::NodeStates& getNodeStates() const;

	Q_PROPERTY(qreal viewDelta READ getViewDelta WRITE setViewDelta)

protected:
	enum
	{
		view_margin = 2,
		update_height = 20,
		value_height = 10,
		event_margin = 4,
		timeline_height = 25,
		width_factor = 500
	};

	void paintEvent(QPaintEvent* event);
	void resizeEvent(QResizeEvent* event);
	void mousePressEvent(QMouseEvent* event);
	void mouseMoveEvent(QMouseEvent* event);
	void mouseReleaseEvent(QMouseEvent* event);
	void wheelEvent(QWheelEvent* event);
	void keyPressEvent(QKeyEvent* event);

	typedef panda::helper::EventData EventData;
	QString eventDescription(const EventData &event);

	qreal posOfTime(long long time);
	qreal lengthOfDuration(long long duration);
	long long timeOfPos(int x);

	QColor getColorForStatus(unsigned int index, qreal s=1.0, qreal v=1.0);

	bool getEventAtPos(QPointF pos, QRectF& rect, const EventData*& pEvent);

	void sortEvents();
	void updateStates(int prevSelection, long long time);
	void setSelectedEvent(int previous, int current);

	long long getComputeDuration(const EventData& event);

	void drawTimeline(QStylePainter& painter);

	void centerViewOnSelection();

	qreal getViewDelta() const;
	void setViewDelta(qreal delta);

	enum MouseAction
	{
		Action_None,
		Action_MovingStart,
		Action_MovingView,
		Action_Zooming
	};

	bool m_valid;
	int m_zoomLevel;
	qreal m_zoomFactor, m_viewDelta;
	QPointF m_previousMousePos, m_currentMousePos;
	MouseAction m_mouseAction;

	typedef panda::helper::UpdateLogger::UpdateEvents UpdateEvents;
	UpdateEvents m_events;	// This list is sorted by the end time of the events
	QVector<unsigned int> m_sortedEvents;	// This list is sorted by the start time of the events (indices into m_events)

	typedef panda::helper::UpdateLogger::NodeStates NodeStates;
	NodeStates m_initialStates, m_currentStates;

	int m_nbThreads;
	QVector<int> m_maxEventLevel, m_startingLevel;
	int m_requiredHeight; // In number of events in the vertical dimension
	long long m_minTime, m_maxTime, m_selectedTime;
	int m_selectedIndex;

	struct EventRect
	{
		EventRect() : event(nullptr) {}
		EventRect(const EventData& e, QRectF r)
			: event(&e), rect(r.adjusted(-1, 0, 1, 0))
		{}

		const EventData* event;
		QRectF rect;
	};
	QVector<EventRect> m_eventRects;
	
signals:
	void setEventText(QString);
	void changedSelectedEvent();
	
public slots:
	void updateEvents();
	void resetZoom();
	void nextEvent();
	void prevEvent();
};

inline const panda::helper::UpdateLogger::NodeStates& UpdateLoggerView::getNodeStates() const
{ return m_currentStates; }

inline qreal UpdateLoggerView::getViewDelta() const
{ return m_viewDelta; }

inline void UpdateLoggerView::setViewDelta(qreal delta)
{ m_viewDelta = delta; update(); }

#endif // UPDATELOGGERDIALOG_H
