#ifndef SCROLLCONTAINER_H
#define SCROLLCONTAINER_H

#include <QAbstractScrollArea>
#include <QPoint>
#include <QPointer>

// For views that we want to scroll, but always occupy the entire scollable area
//  (in QScrollArea, the widget inside is bigger than the container)
class ScrollableView
{
public:
	virtual ~ScrollableView() {}

	virtual QSize viewSize() = 0;
	virtual QPoint viewPosition() = 0;

	virtual void scrollView(QPoint position) = 0;
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

	ScrollableView* m_view;
	QPointer<QWidget> m_viewWidget;
	Qt::Alignment m_alignment;
	mutable QSize m_viewSize;
	QPoint m_deltaPos;
	bool m_updatingScrollValues;
};

#endif // SCROLLCONTAINER_H
