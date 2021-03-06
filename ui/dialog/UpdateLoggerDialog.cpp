#include <ui/dialog/UpdateLoggerDialog.h>

#include <panda/helper/algorithm.h>
#include <vector>

namespace
{

QString getReadableTime(long long time)
{
	auto absTime = abs(time);
	if(absTime >= 1e9)
		return QString("%1s").arg(QString::number(time / 1e9, 'f', 2));
	else if(absTime >= 1e6)
		return QString("%1ms").arg(QString::number(time / 1e6, 'f', 2));
	else if(absTime >= 1e3)
		return QString("%1µs").arg(QString::number(time / 1e3, 'f', 2));
	else
		return QString("%1ns").arg(QString::number(time));
}

QString getLegendText(long long time)
{
	if (!time)
		return "0";

	auto absTime = abs(time);
	if(absTime >= 1e8)
		return QString("%1s").arg(QString::number(time / 1e9));
	else if(absTime >= 1e5)
		return QString("%1ms").arg(QString::number(time / 1e6));
	else if(absTime >= 1e2)
		return QString("%1µs").arg(QString::number(time / 1e3));
	else
		return QString("%1ns").arg(QString::number(time));
}

double nextSmallerFactor(double factor)
{
	int mag = static_cast<int>(floor(log10(factor)));
	int mul = static_cast<int>(factor / pow(10, mag));

	if (mul == 5)
		mul = 2;
	else if (mul == 2)
		mul = 1;
	else if (mul == 1)
	{
		mag -= 1;
		mul = 5;
	}

	return mul * pow(10, mag);
}

}

UpdateLoggerDialog* UpdateLoggerDialog::m_instance = nullptr;

UpdateLoggerDialog::UpdateLoggerDialog(QWidget* parent) :
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

const panda::helper::UpdateLogger::NodeStates& UpdateLoggerDialog::getNodeStates() const
{
	return m_view->getNodeStates();
}

//****************************************************************************//

UpdateLoggerView::UpdateLoggerView(QWidget* parent)
	: QWidget(parent)
	, m_valid(false)
	, m_zoomLevel(0)
	, m_zoomFactor(1.0)
	, m_viewDelta(0.0)
	, m_maxEventLevel(1)
	, m_requiredHeight(0)
	, m_selectedTime(0)
	, m_selectedIndex(-1)
{
	setAutoFillBackground(true);
	setBackgroundRole(QPalette::Light);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setFocusPolicy(Qt::StrongFocus);

	setMouseTracking(true);
}

void UpdateLoggerView::updateEvents()
{
	panda::helper::UpdateLogger* logger = panda::helper::UpdateLogger::getInstance();
	UpdateEvents events;
	m_nbThreads = logger->getNbThreads();
	for(int i=0; i<m_nbThreads; ++i)
		panda::helper::concatenate(events, logger->getEvents(i));

	if(events.empty())
		return;

	m_events = events;
	m_valid = true;

	m_zoomLevel = 0;
	m_zoomFactor = 1.0;
	m_viewDelta = 0.0;

	m_minTime = events.front().m_startTime;
	m_maxTime = events.front().m_endTime;
	m_maxEventLevel.fill(0, m_nbThreads);
	for(auto& event : events)
	{
		if(event.m_startTime < m_minTime) m_minTime = event.m_startTime;
		if(event.m_endTime > m_maxTime) m_maxTime = event.m_endTime;
		if(event.m_level > m_maxEventLevel[event.m_threadId]) m_maxEventLevel[event.m_threadId] = event.m_level;
	}

	m_startingLevel.fill(0, m_nbThreads);
	for(int i=1; i<m_nbThreads; ++i)
		m_startingLevel[i] = m_startingLevel[i-1] + m_maxEventLevel[i-1] + 2;

	m_requiredHeight = 0;
	for(auto l : m_maxEventLevel)
		m_requiredHeight += l + 1;
	m_requiredHeight += m_nbThreads - 1;

	m_selectedTime = m_minTime;
	m_selectedIndex = 0;

	sortEvents();

	setSelectedEvent(0, 0);

	m_currentStates = m_initialStates = logger->getInitialNodeStates();

	updateGeometry();
}

QSize UpdateLoggerView::minimumSizeHint() const
{
	qreal y = timeline_height + view_margin * 2
		+ (m_requiredHeight + 1) * update_height
		+ m_requiredHeight * event_margin;
	y = qMax(static_cast<qreal>(100.0), y);
	return QSize(300, y);
}

QSize UpdateLoggerView::sizeHint() const
{
	return QSize(600, 100);
}

const panda::helper::EventData* UpdateLoggerView::getSelectedEvent() const
{
	if(m_selectedIndex < 0 || m_events.empty() || m_sortedEvents.empty())
		return nullptr;

	return &m_events[m_sortedEvents[m_selectedIndex]];
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

	int width = contentsRect().width();

	QStylePainter painter(this);
	painter.setRenderHint(QPainter::Antialiasing, true);
	painter.setRenderHint(QPainter::TextAntialiasing, true);

	drawTimeline(painter);

	m_eventRects.clear();

	for(int i=m_events.size()-1; i>=0; --i)
	{
		const auto& event = m_events[i];
		qreal x1 = posOfTime(event.m_startTime);
		qreal x2 = posOfTime(event.m_endTime);

		if(x2 < 0 || x1 > width)
			continue;

		qreal y = timeline_height + view_margin + (event.m_level + m_startingLevel[event.m_threadId]) * (update_height + event_margin);

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
			case panda::helper::event_custom:
			{
				QColor c = QColor(196, 196, 196);
				if(event.m_objectIndex > 0)
					c = getColorForStatus(event.m_objectIndex, 0.5);
				painter.setBrush(QBrush(c));
				painter.setPen(c);

				QRectF rect(x1, y, x2 - x1, update_height);
				painter.drawRect(rect);
				m_eventRects.push_back(EventRect(event, rect));
				break;
			}
		}
	}

	QRect viewRect = contentsRect();

	// Threads separation lines
	painter.setPen(QPen(Qt::black, 3));
	for(int i=1; i<m_nbThreads; ++i)
	{
		int eventHeight = (update_height + event_margin);
		int y = timeline_height + (m_startingLevel[i] - 1) * eventHeight + eventHeight/2 - 2;
		painter.drawLine(0, y, viewRect.width(), y);
	}

	// Selection line
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

void UpdateLoggerView::drawTimeline(QStylePainter& painter)
{
	QRect viewRect = contentsRect();
	int w = viewRect.width(), h = viewRect.height();

	painter.setPen(QPen(Qt::black, 2));
	int y = timeline_height - view_margin;
	painter.drawLine(0, y, w, y);

	auto startTime = timeOfPos(0), endTime = timeOfPos(w - 2 * view_margin);
	auto duration = endTime - startTime;
	if (!duration)
		return;

	auto mag = log10(duration);

	// Start with power of ten ticks
	auto tick = pow(10, floor(mag));
	auto len = lengthOfDuration(tick);

	// We want ticks with text approximately every 125-300 px
	double factor = 1;
	if (len > 600)		factor = 0.2;
	else if (len > 250) factor = 0.5;
	else if (len < 125) factor = 2;

	// Draw these ticks black and with text
	painter.setPen(QPen(Qt::black, 1, Qt::PenStyle::DashLine));
	auto ft = factor * tick;
	endTime = std::min<double>(endTime, m_minTime + ceil((m_maxTime - m_minTime) / ft) * ft);
	auto start = m_minTime + ceil((startTime - m_minTime) / ft) * ft;
	for (auto time = start; time <= endTime; time += ft)
	{
		if (time < m_minTime)
			continue;

		auto x = posOfTime(time);
		painter.drawLine(x, timeline_height - 10, x, h);

		auto textArea = QRect(x - 50, 0, 100, timeline_height - 10 - 2);
		painter.drawText(textArea, Qt::AlignHCenter | Qt::AlignBottom, getLegendText(time - m_minTime));
	}

	// Find smaller ticks (at most every 30 px)
	while (true)
	{
		factor = nextSmallerFactor(factor);
		auto tmp = factor * tick;
		len = lengthOfDuration(tmp);

		if (len < 30)
			break;

		ft = tmp;
	}

	// Draw these smaller ticks in gray
	painter.setPen(QPen(Qt::gray, 1, Qt::PenStyle::DotLine));
	start = m_minTime + ceil((startTime - m_minTime) / ft) * ft;
	for (auto time = start; time < endTime; time += ft)
	{
		if (time < m_minTime)
			continue;

		auto x = posOfTime(time);
		painter.drawLine(x, timeline_height - 10, x, h);
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
			m_previousMousePos = m_currentMousePos = event->pos();
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
			long long start = pEvent->m_startTime - m_minTime;
			long long end = pEvent->m_endTime - m_minTime;
			QString times = QString("\n%1ms - %2ms\n(%3 / %4)")
					.arg(start / 1e6).arg(end / 1e6)
					.arg(getReadableTime(getComputeDuration(*pEvent)))
					.arg(getReadableTime(end-start));
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
		if (w > 5)
		{
			m_viewDelta -= x1 / m_zoomFactor; // (using old zoom)
			m_zoomFactor = m_zoomFactor * width_factor / w;
			m_zoomLevel = static_cast<int>(log(m_zoomFactor) / log(1.2) * 10);  // Inverse of "zf = 1.2 ^ (zl / 10)"
		}

		update();
	}
	else if(m_mouseAction == Action_MovingStart)
	{
		if(!m_events.empty())
		{
			long long time = timeOfPos(event->x());
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

			if (m_selectedIndex != prevSelection)
			{
				setSelectedEvent(prevSelection, m_selectedIndex);
				centerViewOnSelection(); // Move the view if the select line goes too far
			}
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
	auto text = QString::fromStdString(event.m_text);
	switch (event.m_type)
	{
	case panda::helper::event_update:		return QString("Update of %1").arg(text);
	case panda::helper::event_getValue:		return QString("GetValue of %1").arg(text);
	case panda::helper::event_render:		return QString("Render of %1").arg(text);
	case panda::helper::event_copyValue:	return QString("Copy to data: %1").arg(text);
	case panda::helper::event_setDirty:		return QString("SetDirty of %1").arg(text);
	default:								return text;
	}
}

qreal UpdateLoggerView::posOfTime(long long time)
{
	qreal a = time - m_minTime;
	qreal b = m_maxTime - m_minTime;
	return view_margin + (m_viewDelta + a / b * width_factor) * m_zoomFactor;
}

qreal UpdateLoggerView::lengthOfDuration(long long duration)
{
	qreal r = m_maxTime - m_minTime;
	return duration * width_factor * m_zoomFactor / r;
}

long long UpdateLoggerView::timeOfPos(int x)
{
	qreal a = (x - view_margin) / m_zoomFactor - m_viewDelta;
	qreal b = m_maxTime - m_minTime;

	return m_minTime + a * b / width_factor;
}

QColor UpdateLoggerView::getColorForStatus(unsigned int index, qreal s, qreal v)
{
	qreal h = index * 0.13; // randomly chosen so that we have a palette of possible colors
	h = h - floor(h);
	return QColor::fromHsvF(h, s, v);
}

bool UpdateLoggerView::getEventAtPos(QPointF pos, QRectF& rect, const EventData*& pEvent)
{
	for(int i=m_eventRects.size()-1; i>=0; --i)
	{
		const auto& er = m_eventRects[i];
		if(er.rect.contains(pos))
		{
			pEvent = er.event;
			rect = er.rect;
			return true;
		}
	}
	return false;
}

void UpdateLoggerView::sortEvents()
{
	std::sort(m_events.begin(), m_events.end(), [](const EventData& lhs, const EventData& rhs){
		return lhs.m_endTime < rhs.m_endTime;
	});

	int nb = m_events.size();
	m_sortedEvents.resize(nb);
	for(int i=0; i<nb; ++i)
		m_sortedEvents[i] = i;

	std::sort(m_sortedEvents.begin(), m_sortedEvents.end(), [this](const unsigned int& lhs, const unsigned int& rhs){
		return m_events[lhs].m_startTime < m_events[rhs].m_startTime;
	});
}

void UpdateLoggerView::prevEvent()
{
	if(m_events.empty())
		return;

	int prevSelection = m_selectedIndex;
	m_selectedIndex = qMax(0, m_selectedIndex-1);

	if(m_selectedIndex != prevSelection)
	{
		setSelectedEvent(prevSelection, m_selectedIndex);
		centerViewOnSelection(); // Move the view if the select line goes too far
	}
}

void UpdateLoggerView::nextEvent()
{
	if(m_events.empty())
		return;

	int prevSelection = m_selectedIndex;
	m_selectedIndex = qMin(m_sortedEvents.size()-1, m_selectedIndex+1);

	if(m_selectedIndex != prevSelection)
	{
		setSelectedEvent(prevSelection, m_selectedIndex);
		centerViewOnSelection(); // Move the view if the select line goes too far
	}
}

void UpdateLoggerView::updateStates(int prevSelection, long long time)
{
	if(prevSelection < 0)
		prevSelection = 0;

	long long prevTime = m_events[m_sortedEvents[prevSelection]].m_startTime;

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

long long UpdateLoggerView::getComputeDuration(const EventData& event)
{
	for(int index = (&event - &m_events[0]) - 1; index > 0; --index)
	{
		const EventData& other = m_events[index];
		if(other.m_threadId == event.m_threadId)
		{
			if(other.m_endTime > event.m_endTime)
				break;
			return (event.m_endTime - other.m_endTime);
		}
	}

	return (event.m_endTime - event.m_startTime);
}

void UpdateLoggerView::setSelectedEvent(int previous, int current)
{
	const EventData& event = m_events[m_sortedEvents[m_selectedIndex]];
	m_selectedTime = event.m_startTime;
	updateStates(previous, m_selectedTime);

	QString display = QString("%1 %2")
		.arg(getReadableTime(m_selectedTime - m_minTime), -10)
		.arg(eventDescription(event));
	emit setEventText(display);
	emit changedSelectedEvent();

	update();
}

void UpdateLoggerView::centerViewOnSelection()
{
	int x = posOfTime(m_selectedTime);
	int w = width();
	if (x < 0.1 * w || x > 0.9 * w)
	{
		qreal a = m_selectedTime - m_minTime;
		qreal b = m_maxTime - m_minTime;
		qreal newDelta = 0.5 * w / m_zoomFactor - a / b * width_factor;

		// Animate the movement of the view (easier to understand for the user)
		auto anim = new QPropertyAnimation(this, "viewDelta");
		anim->setDuration(300);
		anim->setEndValue(newDelta);
		anim->setEasingCurve(QEasingCurve::InOutQuad);
		anim->start(QAbstractAnimation::DeleteWhenStopped);
	}
}
