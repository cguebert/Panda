#include <ui/ScrollContainer.h>

#include <QEvent>
#include <QLayout>
#include <QScrollBar>
#include <QStyle>

ScrollContainer::ScrollContainer(QWidget* parent)
	: QAbstractScrollArea(parent)
	, m_view(nullptr)
	, m_alignment(0)
	, m_updatingScrollValues(false)
{
	viewport()->setBackgroundRole(QPalette::NoRole);
	horizontalScrollBar()->setSingleStep(20);
	verticalScrollBar()->setSingleStep(20);
}

ScrollContainer::~ScrollContainer()
{
}

void ScrollContainer::setView(ScrollableView* view)
{
	if (view == m_view || !view)
		return;

	if(m_view)
		delete m_view;

	m_view = view;
	m_viewWidget = dynamic_cast<QWidget*>(view);

	horizontalScrollBar()->setValue(0);
	verticalScrollBar()->setValue(0);
	if (m_viewWidget->parentWidget() != viewport())
		m_viewWidget->setParent(viewport());
	if (!m_viewWidget->testAttribute(Qt::WA_Resized))
		m_viewWidget->resize(m_viewWidget->sizeHint());

	m_viewWidget->setAutoFillBackground(true);
	m_viewWidget->installEventFilter(this);
	m_viewSize = QSize();
	updateScrollBars();
	m_viewWidget->show();

	connect(m_viewWidget, SIGNAL(viewModified()), this, SLOT(viewModified()));
}

ScrollableView* ScrollContainer::view() const
{
	return m_view;
}

ScrollableView* ScrollContainer::takeView()
{
	ScrollableView* view = m_view;
	m_view = nullptr;
	if (m_viewWidget)
		m_viewWidget->setParent(nullptr);
	m_viewWidget = nullptr;
	return view;
}

void ScrollContainer::setAlignment(Qt::Alignment alignment)
{
	m_alignment = alignment;
	if (m_view)
		updateViewPosition();
}

Qt::Alignment ScrollContainer::alignment() const
{
	return m_alignment;
}

QSize ScrollContainer::sizeHint() const
{
	int f = 2 * frameWidth();
	QSize sz(f, f);
	int h = fontMetrics().height();
	if (m_view)
	{
		if (!m_viewSize.isValid())
			m_viewSize = m_viewWidget->sizeHint();
		sz += m_viewSize;
	}
	else
		sz += QSize(12 * h, 8 * h);

	if (horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOn)
		sz.setHeight(sz.height() + horizontalScrollBar()->sizeHint().height());
	if (verticalScrollBarPolicy() == Qt::ScrollBarAlwaysOn)
		sz.setWidth(sz.width() + verticalScrollBar()->sizeHint().width());
	return sz.boundedTo(QSize(36 * h, 24 * h));
}

QSize ScrollContainer::viewportSizeHint() const
{
	if (m_viewWidget)
		return m_viewWidget->sizeHint();

	const int h = fontMetrics().height();
	return QSize(6 * h, 4 * h);
}

void ScrollContainer::updateViewPosition()
{
	QRect viewportRect = viewport()->rect();
	QSize viewSize = m_view->viewSize();
	QPoint scrolled = QPoint(-horizontalScrollBar()->value(), -verticalScrollBar()->value());
	QRect aligned = QStyle::alignedRect(Qt::LeftToRight, m_alignment, viewSize, viewportRect);

	QPoint targetPos;
	if(!m_alignment)
		targetPos = scrolled + m_deltaPos;
	else
		targetPos = QPoint(viewSize.width() < viewportRect.width() ? aligned.x() : scrolled.x(),
						   viewSize.height() < viewportRect.height() ? aligned.y() : scrolled.y());
	if(targetPos != m_view->viewPosition())
	{
		m_view->scrollView(targetPos);
		updateScrollBars();
	}
}

void ScrollContainer::updateScrollBars()
{
	if (!m_view)
		return;

	m_updatingScrollValues = true;

	QSize viewportSize = viewport()->size();
	QSize viewSize = m_view->viewSize();
	QPoint pos = m_view->viewPosition();

	int dw = std::max(0, viewportSize.width() - viewSize.width() - pos.x()); // space from the right of the view to the right of the viewport
	int w = std::max(0, viewSize.width() - viewportSize.width() + dw + std::max(0, pos.x()));
	int x = std::max(0, -pos.x());
	horizontalScrollBar()->setRange(0, w);
	horizontalScrollBar()->setPageStep(viewportSize.width());
	horizontalScrollBar()->setValue(x);

	int dh = std::max(0, viewportSize.height() - viewSize.height() - pos.y());
	int h = std::max(0, viewSize.height() - viewportSize.height() + dh + std::max(0, pos.y()));
	int y = std::max(0, -pos.y());
	verticalScrollBar()->setRange(0, h);
	verticalScrollBar()->setPageStep(viewportSize.height());
	verticalScrollBar()->setValue(y);

	m_deltaPos = QPoint(std::max(0, pos.x()), std::max(0, pos.y()));

	m_updatingScrollValues = false;
}

void ScrollContainer::resizeView()
{
	if (!m_view)
		return;

	QSize viewportSize = viewport()->size();
	QSize maxSize = maximumViewportSize();

	QSize viewSize = m_view->viewSize();
	QPoint pos = m_view->viewPosition();
	QSize size;
	if(pos.x() > 0)
		size.setWidth(viewSize.width() + pos.x());
	else
		size.setWidth(std::max(viewSize.width(), viewportSize.width()) - pos.x());
	if(pos.y() > 0)
		size.setHeight(viewSize.height() + pos.y());
	else
		size.setHeight(std::max(viewSize.height(), viewportSize.height()) - pos.y());

	if(size.width() < maxSize.width() && horizontalScrollBarPolicy() != Qt::ScrollBarAlwaysOn)
		viewportSize.setHeight(maxSize.height());
	if(size.height() < maxSize.height() && verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOn)
		viewportSize.setWidth(maxSize.width());
	m_viewWidget->resize(viewportSize);

	updateScrollBars();
}

bool ScrollContainer::event(QEvent *e)
{
	if (e->type() == QEvent::StyleChange || e->type() == QEvent::LayoutRequest)
		resizeView();
	return QAbstractScrollArea::event(e);
}

void ScrollContainer::resizeEvent(QResizeEvent *)
{
	resizeView();
}
void ScrollContainer::scrollContentsBy(int, int)
{
	if (!m_view || m_updatingScrollValues)
		return;

	updateViewPosition();
}

void ScrollContainer::viewModified()
{
	resizeView();
}
