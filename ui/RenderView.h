#ifndef RENDERVIEW_H
#define RENDERVIEW_H

#include <QWidget>

namespace panda
{
class PandaDocument;
class PandaObject;
class BaseData;
}


class RenderView : public QWidget
{
	Q_OBJECT
public:
	explicit RenderView(panda::PandaDocument* doc, QWidget *parent = 0);

	QSize minimumSizeHint() const;
	QSize sizeHint() const;
	
protected:
	void paintEvent(QPaintEvent *event);

signals:
	
public slots:

private:
	panda::PandaDocument* pandaDocument;
};

#endif // RENDERVIEW_H
