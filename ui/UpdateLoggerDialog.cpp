#include <ui/UpdateLoggerDialog.h>

#include <QVector>
#include <algorithm>

UpdateLoggerDialog::UpdateLoggerDialog(QWidget *parent) :
    QDialog(parent)
{
	m_view = new UpdateLoggerView(this);

	QPushButton* resetZoomButton = new QPushButton("Reset zoom");
	QPushButton* updateButton = new QPushButton("Update");
    QPushButton* okButton = new QPushButton("Ok");
    QHBoxLayout* buttonsLayout = new QHBoxLayout;

    buttonsLayout->addStretch();
	buttonsLayout->addWidget(resetZoomButton);
	buttonsLayout->addWidget(updateButton);
    buttonsLayout->addWidget(okButton);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_view);
    mainLayout->addItem(buttonsLayout);

    setLayout(mainLayout);

	connect(resetZoomButton, SIGNAL(clicked()), m_view, SLOT(resetZoom()));
	connect(updateButton, SIGNAL(clicked()), m_view, SLOT(updateEvents()));
	connect(okButton, SIGNAL(clicked()), this, SLOT(hide()));

    m_view->setFocus();
}

void UpdateLoggerDialog::updateEvents()
{
	m_view->updateEvents();
}

//***************************************************************//

UpdateLoggerView::UpdateLoggerView(QWidget *parent)
    : QWidget(parent)
    , m_valid(false)
    , m_zoomLevel(0)
    , m_zoomFactor(1.0)
    , m_viewDelta(0.0)
	, m_maxEventLevel(1)
{
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Light);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setFocusPolicy(Qt::StrongFocus);

    setMouseTracking(true);

	m_tps = 1000.0 / panda::helper::UpdateLogger::getTicksPerSec();
}

void UpdateLoggerView::updateEvents()
{
	UpdateEvents events = panda::helper::UpdateLogger::getInstance()->getEvents();
	if(events.isEmpty())
		return;

	m_events = events;
	m_valid = true;

	m_zoomLevel = 0;
	m_zoomFactor = 1.0;
	m_viewDelta = 0.0;

	m_minTime = events.front().m_start;
	m_maxTime = events.front().m_end;
	m_maxEventLevel = 0;
	for(auto& event : events)
	{
		if(event.m_start < m_minTime) m_minTime = event.m_end;
		if(event.m_end > m_maxTime) m_maxTime = event.m_end;
		if(event.m_level > m_maxEventLevel) m_maxEventLevel = event.m_level;
	}

	setMinimumSize(minimumSizeHint());

	update();
}

QSize UpdateLoggerView::minimumSizeHint() const
{
    qreal y = view_margin * 2
		+ (m_maxEventLevel + 1) * update_height
		+ m_maxEventLevel * event_margin;
    y = qMax(static_cast<qreal>(100.0), y);
    return QSize(300, y);
}

QSize UpdateLoggerView::sizeHint() const
{
    return QSize(600, 100);
}

void UpdateLoggerView::resetZoom()
{
	m_zoomLevel = 0;
	m_zoomFactor = 1.0;
	m_viewDelta = 0;
	update();
}

void UpdateLoggerView::paintEvent(QPaintEvent*)
{
    if(!m_valid)
        return;

    QStylePainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    m_eventRects.clear();

//	for(const auto& event : m_events)
	QVectorIterator<EventData> iter(m_events);
	iter.toBack();
	while(iter.hasPrevious())
	{
		const EventData& event = iter.previous();
		qreal x1 = posOfTime(event.m_start);
		qreal x2 = posOfTime(event.m_end);
		qreal y = view_margin + event.m_level * (update_height + event_margin);

		switch (event.m_type)
		{
			case panda::helper::event_update:
			case panda::helper::event_render:
			{
				QColor c = getColorForStatus(event.m_index, 0.5);
				painter.setBrush(QBrush(c));
				painter.setPen(c);

				QRectF rect(x1, y, x2 - x1, update_height);
				painter.drawRect(rect);
				m_eventRects.push_back(EventRect(event, rect));
				break;
			}
			case panda::helper::event_getValue:
			{
				QColor c = getColorForStatus(event.m_index);
				painter.setBrush(QBrush(c));
				painter.setPen(c);

				QRectF rect(x1, y + (update_height-value_height), x2 - x1, value_height);
				painter.drawRect(rect);
				m_eventRects.push_back(EventRect(event, rect));
				break;
			}
			case panda::helper::event_copyValue:
			{
				QColor c = getColorForStatus(event.m_index, 1, 0.5);
				painter.setBrush(QBrush(c));
				painter.setPen(c);

				QRectF rect(x1, y + (update_height-value_height), x2 - x1, value_height);
				painter.drawRect(rect);
				m_eventRects.push_back(EventRect(event, rect));
				break;
			}
		}
	}

	if (m_mouseAction == Action_Zooming)
	{
		QRectF zoomRect(m_previousMousePos, m_currentMousePos);
		QBrush brush(Qt::NoBrush);
		QPen pen(palette().text().color());
		pen.setStyle(Qt::DashDotLine);
		painter.setPen(pen);
		painter.setBrush(brush);
		painter.drawRect(zoomRect);
	}
}

void UpdateLoggerView::resizeEvent(QResizeEvent*)
{
    update();
}

void UpdateLoggerView::mousePressEvent(QMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
    {
		if (event->modifiers() == Qt::ControlModifier)
		{
			m_previousMousePos = event->pos();
			m_mouseAction = Action_Zooming;
		}
		else
		{
			m_previousMousePos = m_currentMousePos = event->pos();
			m_mouseAction = Action_MovingView;
		}
    }
}

void UpdateLoggerView::mouseMoveEvent(QMouseEvent* event)
{
    if(m_mouseAction == Action_MovingView)
    {
		QPointF delta = (event->pos() - m_previousMousePos) / m_zoomFactor;
        m_viewDelta += delta.x();
		m_previousMousePos = event->pos();
        update();
    }
	else if (m_mouseAction == Action_Zooming)
	{
		m_currentMousePos = event->pos();
		update();
	}
    else if(m_mouseAction == Action_None)
    {
        QRectF rect;
		const EventData* pEvent = nullptr;
		if(getEventAtPos(event->localPos(), rect, pEvent))
        {
			qreal start = (pEvent->m_start - m_minTime) * m_tps;
			qreal end = (pEvent->m_end - m_minTime) * m_tps;
			QString times = QString("\n%1ms - %2ms").arg(start).arg(end);
			QString display;
			switch (pEvent->m_type)
			{
				case panda::helper::event_update:	{ display = QString("Update of %1").arg(pEvent->m_objectName); break; }
				case panda::helper::event_getValue: { display = QString("%1 asked the value of %2").arg(pEvent->m_objectName).arg(pEvent->m_dataName); break; }
				case panda::helper::event_render:	{ display = QString("Render of %1").arg(pEvent->m_objectName); break; }
				case panda::helper::event_copyValue: { display = QString("Copy to data: %1").arg(pEvent->m_dataName); break; }
			}

			if(!display.isEmpty())
				QToolTip::showText(event->globalPos(), display + times, this, rect.toRect());
		}
    }
}

void UpdateLoggerView::mouseReleaseEvent(QMouseEvent*)
{
	if (m_mouseAction == Action_Zooming)
	{
		int x1 = std::min(m_previousMousePos.x(), m_currentMousePos.x());
		int x2 = std::max(m_previousMousePos.x(), m_currentMousePos.x());
		qreal w = x2 - x1;
		qreal wc = width() - 2 * view_margin;

		m_viewDelta -= x1 / m_zoomFactor; // (using old zoom)
		m_zoomFactor *= wc / w;
		m_zoomLevel = static_cast<int>(log(m_zoomFactor) / log(1.2) * 10);  // Inverse of "zf = 1.2 ^ (zl / 10)"

		update();
	}

    m_mouseAction = Action_None;
}

void UpdateLoggerView::wheelEvent(QWheelEvent* event)
{
	int ticks = event->delta() / 15;
	int newZoom = qMax(0, m_zoomLevel+ticks);
    if(m_zoomLevel != newZoom)
    {
        qreal mouseX = event->pos().x();
        qreal oldX = mouseX / m_zoomFactor;
        m_zoomLevel = newZoom;
		m_zoomFactor = pow( 1.2, m_zoomLevel / 10.0 );  // Note: if changing the value 1.2, also change it in the function above
        if(!m_zoomLevel)
            m_viewDelta = 0.0;
        else
            m_viewDelta += mouseX / m_zoomFactor - oldX;
        update();
    }
}

void UpdateLoggerView::keyPressEvent(QKeyEvent* event)
{
	if(event->key() == Qt::Key_1)
		resetZoom();
}

qreal UpdateLoggerView::posOfTime(unsigned long long time)
{
    qreal w = width() - 2 * view_margin;
	qreal a = time - m_minTime;
	qreal b = m_maxTime - m_minTime;
	return view_margin + (m_viewDelta + a / b * w) * m_zoomFactor;
}

QColor UpdateLoggerView::getColorForStatus(unsigned int index, qreal s, qreal v)
{
	qreal h = index * 0.13; // randomly chosen so that we have a palette of possible colors
	h = h - floor(h);
    return QColor::fromHsvF(h, s, v);
}

bool UpdateLoggerView::getEventAtPos(QPointF pos, QRectF& rect, const EventData*& pEvent)
{
	QVectorIterator<EventRect> iter(m_eventRects);
	iter.toBack();
	while(iter.hasPrevious())
    {
		const EventRect& er = iter.previous();
        if(er.rect.contains(pos))
        {
			pEvent = er.event;
            rect = er.rect;
            return true;
        }
    }
    return false;
}

