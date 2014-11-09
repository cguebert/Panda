#ifndef SCROLLCONTAINER_H
#define SCROLLCONTAINER_H

#include <QAbstractScrollArea>
#include <QPoint>
#include <QPointer>

// For views that we want to scroll, but always occupy the entire scollable area
//  (in QScrollArea, the widget inside is bigger than the container)
class ScrollableView : public QWidget
{
	Q_OBJECT
public:
	explicit ScrollableView(QWidget* parent = nullptr) : QWidget(parent) {}
	~ScrollableView() {}

	virtual QSize viewSize() = 0;
	virtual QPoint viewPosition() = 0;

	virtual void scrollView(QPoint position) = 0;

signals:
	void viewModified();
};

class ScrollContainer : public QAbstractScrollArea
{
	Q_OBJECT
public:

	explicit ScrollContainer(QWidget* parent = nullptr);
	~ScrollContainer();

	void setView(ScrollableView* view);
	ScrollableView* view() const;
	ScrollableView* takeView();

	Qt::Alignment alignment() const;
	void setAlignment(Qt::Alignment);

	QSize sizeHint() const;

public slots:
	void viewModified();

protected:
	QSize viewportSizeHint() const;

	void updateScrollBars();
	void updateViewPosition();
	void resizeView();

	bool event(QEvent*);
	void resizeEvent(QResizeEvent*);
	void scrollContentsBy(int dx, int dy);

	QPointer<ScrollableView> m_view;
	Qt::Alignment m_alignment;
	mutable QSize m_viewSize;
	mutable QPoint m_deltaPos;
};

#endif // SCROLLCONTAINER_H
