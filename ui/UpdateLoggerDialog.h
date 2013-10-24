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
	explicit UpdateLoggerDialog(QWidget *parent = 0);
	void updateEvents();

protected:
	UpdateLoggerView* m_view;

};

//***************************************************************//

class UpdateLoggerView : public QWidget
{
    Q_OBJECT
public:
	explicit UpdateLoggerView(QWidget *parent = 0);

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

protected:
    enum
    {
        view_margin = 2,
		update_height = 20,
		value_height = 10,
		event_margin = 4
    };

    void paintEvent(QPaintEvent *event);
    void resizeEvent(QResizeEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);
    void keyPressEvent(QKeyEvent *event);

	inline qreal posOfTime(unsigned long long time);
    QColor getColorForStatus(unsigned int index, qreal s=1.0, qreal v=1.0);

	typedef panda::helper::EventData EventData;
	bool getEventAtPos(QPointF pos, QRectF& rect, const EventData*& pEvent);

    enum MouseAction
    {
        Action_None,
		Action_MovingView,
		Action_Zooming
    };

    bool m_valid;
    int m_zoomLevel;
    qreal m_zoomFactor, m_viewDelta;
	QPointF m_previousMousePos, m_currentMousePos;
    MouseAction m_mouseAction;

	typedef panda::helper::UpdateLogger::UpdateEvents UpdateEvents;
	UpdateEvents m_events;
	quint32 m_maxEventLevel;
	unsigned long long m_minTime, m_maxTime;
	qreal m_tps;

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
    
public slots:
	void updateEvents();
	void resetZoom();
};

#endif // UPDATELOGGERDIALOG_H
