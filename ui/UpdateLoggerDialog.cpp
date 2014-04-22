#include <ui/UpdateLoggerDialog.h>

#include <QVector>
#include <algorithm>

UpdateLoggerDialog* UpdateLoggerDialog::m_instance = nullptr;

UpdateLoggerDialog::UpdateLoggerDialog(QWidget *parent) :
    QDialog(parent)
{
	m_view = new UpdateLoggerView(this);

	QPushButton* prevEventButton = new QPushButton("Previous");
	QPushButton* nextEventButton = new QPushButton("Next");
	QPushButton* resetZoomButton = new QPushButton("Reset zoom");
	QPushButton* updateButton = new QPushButton("Update");
    QPushButton* okButton = new QPushButton("Ok");
    QHBoxLayout* buttonsLayout = new QHBoxLayout;

	buttonsLayout->addWidget(prevEventButton);
	buttonsLayout->addWidget(nextEventButton);
    buttonsLayout->addStretch();
	buttonsLayout->addWidget(resetZoomButton);
	buttonsLayout->addWidget(updateButton);
    buttonsLayout->addWidget(okButton);

	m_label = new QLabel(this);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_view);
	mainLayout->addWidget(m_label);
    mainLayout->addItem(buttonsLayout);

    setLayout(mainLayout);

	connect(m_view, SIGNAL(setEventText(QString)), this, SLOT(setEventText(QString)));
	connect(prevEventButton, SIGNAL(clicked()), m_view, SLOT(prevEvent()));
	connect(nextEventButton, SIGNAL(clicked()), m_view, SLOT(nextEvent()));
	connect(resetZoomButton, SIGNAL(clicked()), m_view, SLOT(resetZoom()));
	connect(updateButton, SIGNAL(clicked()), m_view, SLOT(updateEvents()));
	connect(okButton, SIGNAL(clicked()), this, SLOT(hide()));

	connect(m_view, SIGNAL(changedSelectedEvent()), this, SIGNAL(changedSelectedEvent()));

    m_view->setFocus();
}

void UpdateLoggerDialog::updateEvents()
{
	m_view->updateEvents();
	emit changedSelectedEvent();
}

void UpdateLoggerDialog::setEventText(QString text)
{
	m_label->setText(text);
}

UpdateLoggerDialog* UpdateLoggerDialog::getInstance()
{
	return m_instance;
}

void UpdateLoggerDialog::setInstance(UpdateLoggerDialog* dlg)
{
	m_instance = dlg;
}

const panda::helper::EventData* UpdateLoggerDialog::getSelectedEvent() const
{
	return m_view->getSelectedEvent();
}

bool UpdateLoggerDialog::isNodeDirty(const panda::DataNode* node) const
{
	return m_view->isNodeDirty(node);
}

//***************************************************************//

UpdateLoggerView::UpdateLoggerView(QWidget *parent)
    : QWidget(parent)
    , m_valid(false)
    , m_zoomLevel(0)
    , m_zoomFactor(1.0)
    , m_viewDelta(0.0)
	, m_maxEventLevel(1)
	, m_selectedTime(0)
	, m_selectedIndex(-1)
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
	panda::helper::UpdateLogger* logger = panda::helper::UpdateLogger::getInstance();
	UpdateEvents events = logger->getEvents(0);
	if(events.isEmpty())
		return;

	m_events = events;
	m_valid = true;

	m_zoomLevel = 0;
	m_zoomFactor = 1.0;
	m_viewDelta = 0.0;

	m_minTime = events.front().m_startTime;
	m_maxTime = events.front().m_endTime;
	m_maxEventLevel = 0;
	for(auto& event : events)
	{
		if(event.m_startTime < m_minTime) m_minTime = event.m_startTime;
		if(event.m_endTime > m_maxTime) m_maxTime = event.m_endTime;
		if(event.m_level > m_maxEventLevel) m_maxEventLevel = event.m_level;
	}

	m_selectedTime = m_minTime;
	m_selectedIndex = 0;

	sortEvents();

	const EventData& event = m_events[m_sortedEvents[m_selectedIndex]];
	QString display = eventDescription(event);
	emit setEventText(display);

	m_currentStates = m_initialStates = logger->getInitialNodeStates();

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

const panda::helper::EventData* UpdateLoggerView::getSelectedEvent() const
{
	if(m_selectedIndex < 0 || m_events.isEmpty() || m_sortedEvents.isEmpty())
		return nullptr;

	return &m_events[m_sortedEvents[m_selectedIndex]];
}

bool UpdateLoggerView::isNodeDirty(const panda::DataNode* node) const
{
	return m_currentStates[node];
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

	QVectorIterator<EventData> iter(m_events);
	iter.toBack();
	while(iter.hasPrevious())
	{
		const EventData& event = iter.previous();
		qreal x1 = posOfTime(event.m_startTime);
		qreal x2 = posOfTime(event.m_endTime);
		qreal y = view_margin + event.m_level * (update_height + event_margin);

		switch (event.m_type)
		{
			case panda::helper::event_update:
			case panda::helper::event_render:
			{
				QColor c = getColorForStatus(event.m_objectIndex, 0.5);
				painter.setBrush(QBrush(c));
				painter.setPen(c);

				QRectF rect(x1, y, x2 - x1, update_height);
				painter.drawRect(rect);
				m_eventRects.push_back(EventRect(event, rect));
				break;
			}
			case panda::helper::event_getValue:
			{
				QColor c = getColorForStatus(event.m_objectIndex);
				painter.setBrush(QBrush(c));
				painter.setPen(c);

				QRectF rect(x1, y + (update_height-value_height), x2 - x1, value_height);
				painter.drawRect(rect);
				m_eventRects.push_back(EventRect(event, rect));
				break;
			}
			case panda::helper::event_copyValue:
			{
				QColor c = getColorForStatus(event.m_objectIndex, 1, 0.5);
				painter.setBrush(QBrush(c));
				painter.setPen(c);

				QRectF rect(x1, y + (update_height-value_height), x2 - x1, value_height);
				painter.drawRect(rect);
				m_eventRects.push_back(EventRect(event, rect));
				break;
			}
			case panda::helper::event_setDirty:
			{
				QColor c = getColorForStatus(event.m_objectIndex);
				painter.setBrush(QBrush(c));
				painter.setPen(c);

				QRectF rect(x1, y, x2 - x1, value_height);
				painter.drawRect(rect);
				m_eventRects.push_back(EventRect(event, rect));
				break;
			}
		}
	}

	// Selection line
	QRect viewRect = contentsRect();
	int x = posOfTime(m_selectedTime);
	painter.setPen(Qt::red);
	painter.drawLine(x, 0, x, viewRect.height());

	// Zoom rectangle
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
			m_mouseAction = Action_MovingStart;
		}
    }
}

void UpdateLoggerView::mouseMoveEvent(QMouseEvent* event)
{
	if(m_mouseAction == Action_MovingStart)
	{
		int x = (event->x() - m_previousMousePos.x());
		if(qAbs(x) > 5)
		{
			m_mouseAction = Action_MovingView;
			m_viewDelta += x / m_zoomFactor;
			m_previousMousePos = event->pos();
			update();
		}
	}
	else if(m_mouseAction == Action_MovingView)
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
			qreal start = (pEvent->m_startTime - m_minTime) * m_tps;
			qreal end = (pEvent->m_endTime - m_minTime) * m_tps;
			QString times = QString("\n%1ms - %2ms").arg(start).arg(end);
			QString display = eventDescription(*pEvent);

			if(!display.isEmpty())
				QToolTip::showText(event->globalPos(), display + times, this, rect.toRect());
		}
    }
}

void UpdateLoggerView::mouseReleaseEvent(QMouseEvent* event)
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
	else if(m_mouseAction == Action_MovingStart)
	{
		if(!m_events.isEmpty())
		{
			unsigned long long time = timeOfPos(event->x());
			int nb = m_sortedEvents.size();
			int prevSelection = m_selectedIndex;
			m_selectedIndex = 0;
			for(int i=0; i<nb; ++i)
			{
				const EventData& event = m_events[m_sortedEvents[i]];
				if(event.m_startTime < time)
					m_selectedIndex = i;
				else
					break;
			}

			if(m_selectedIndex != prevSelection)
			{
				const EventData& event = m_events[m_sortedEvents[m_selectedIndex]];
				m_selectedTime = event.m_startTime;
				updateStates(prevSelection, m_selectedTime);

				QString display = eventDescription(event);
				emit setEventText(display);
				emit changedSelectedEvent();
			}

			update();
		}
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

QString UpdateLoggerView::eventDescription(const EventData& event)
{
	QString display;
	switch (event.m_type)
	{
		case panda::helper::event_update:		{ display = QString("Update of %1").arg(event.m_objectName);	break;	}
		case panda::helper::event_getValue:		{ display = QString("GetValue of %1").arg(event.m_dataName);	break;	}
		case panda::helper::event_render:		{ display = QString("Render of %1").arg(event.m_objectName);	break;	}
		case panda::helper::event_copyValue:	{ display = QString("Copy to data: %1").arg(event.m_dataName);	break;	}
		case panda::helper::event_setDirty:
		{
			if(!event.m_dataName.isEmpty())
				display = QString("SetDirty of %1 / %2").arg(event.m_objectName).arg(event.m_dataName);
			else
				display = QString("SetDirty of %1").arg(event.m_objectName);
			break;
		}
	}

	return display;
}

qreal UpdateLoggerView::posOfTime(unsigned long long time)
{
    qreal w = width() - 2 * view_margin;
	qreal a = time - m_minTime;
	qreal b = m_maxTime - m_minTime;
	return view_margin + (m_viewDelta + a / b * w) * m_zoomFactor;
}

unsigned long long UpdateLoggerView::timeOfPos(int x)
{
	qreal w = width() - 2 * view_margin;
	qreal a = (x - view_margin) / m_zoomFactor - m_viewDelta;
	qreal b = m_maxTime - m_minTime;

	return m_minTime + a * b / w;
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

class EventsComparator
{
public:
	EventsComparator(panda::helper::UpdateLogger::UpdateEvents& events)
		: m_events(events)
	{}

	bool operator()(const int& lhs, const int& rhs)
	{
		return m_events[lhs].m_startTime < m_events[rhs].m_startTime;
	}

protected:
	panda::helper::UpdateLogger::UpdateEvents& m_events;
};

void UpdateLoggerView::sortEvents()
{
	int nb = m_events.size();
	m_sortedEvents.resize(nb);
	for(int i=0; i<nb; ++i)
		m_sortedEvents[i] = i;

	EventsComparator comparator(m_events);
	qSort(m_sortedEvents.begin(), m_sortedEvents.end(), comparator);
}

void UpdateLoggerView::prevEvent()
{
	if(m_events.isEmpty())
		return;

	int prevSelection = m_selectedIndex;
	m_selectedIndex = qMax(0, m_selectedIndex-1);

	if(m_selectedIndex != prevSelection)
	{
		const EventData& event = m_events[m_sortedEvents[m_selectedIndex]];
		m_selectedTime = event.m_startTime;

		updateStates(prevSelection, m_selectedTime);

		// Move the view if the select line goes too far
		int x = posOfTime(m_selectedTime);
		int w = width();
		if(x < 0.1 * w || x > w - view_margin)
		{
			qreal a = m_selectedTime - m_minTime;
			qreal b = m_maxTime - m_minTime;
			qreal c = w - 2 * view_margin;
			m_viewDelta = 0.5 * w / m_zoomFactor - a / b * c;
		}

		QString display = eventDescription(event);
		emit setEventText(display);
		emit changedSelectedEvent();

		update();
	}
}

void UpdateLoggerView::nextEvent()
{
	if(m_events.isEmpty())
		return;

	int prevSelection = m_selectedIndex;
	m_selectedIndex = qMin(m_sortedEvents.size()-1, m_selectedIndex+1);

	if(m_selectedIndex != prevSelection)
	{
		const EventData& event = m_events[m_sortedEvents[m_selectedIndex]];
		m_selectedTime = event.m_startTime;

		updateStates(prevSelection, m_selectedTime);

		// Move the view if the select line goes too far
		int x = posOfTime(m_selectedTime);
		int w = width();
		if(x > 0.9 * w || x < view_margin)
		{
			qreal a = m_selectedTime - m_minTime;
			qreal b = m_maxTime - m_minTime;
			qreal c = w - 2 * view_margin;
			m_viewDelta = 0.5 * w / m_zoomFactor - a / b * c;
		}

		QString display = eventDescription(event);
		emit setEventText(display);
		emit changedSelectedEvent();

		update();
	}
}

void UpdateLoggerView::updateStates(int prevSelection, unsigned long long time)
{
	if(prevSelection < 0)
		prevSelection = 0;

	unsigned long long prevTime = m_events[m_sortedEvents[prevSelection]].m_startTime;

	if(time > prevTime)
	{
		for(int i=m_sortedEvents[prevSelection]; m_events[i].m_endTime < time; ++i)
			m_currentStates[m_events[i].m_node] = m_events[i].m_dirtyEnd;
	}
	else if(time < prevTime)
	{
		for(int i=prevSelection; m_events[m_sortedEvents[i]].m_startTime > time; --i)
			m_currentStates[m_events[m_sortedEvents[i]].m_node] = m_events[m_sortedEvents[i]].m_dirtyStart;
	}
}
