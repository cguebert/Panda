#ifndef LINKTAG_H
#define LINKTAG_H

#include <QRectF>
#include <QMap>

namespace panda
{
class BaseData;
}

class GraphView;

class LinkTag
{
public:
	LinkTag(GraphView* view, panda::BaseData* input, panda::BaseData* output);
	void addOutput(panda::BaseData* output);
	void removeOutput(panda::BaseData* output);
	void update();
	bool isEmpty();
	bool isHovering(const QPointF& point);

	void moveView(const QPointF& delta);
	void draw(QPainter* painter);

	panda::BaseData* getInputData();

	int index;
	bool hovering;

	static const int tagW = 20;
	static const int tagH = 11;
	static const int tagMargin = 10;
protected:
	GraphView* parentView;
	panda::BaseData* inputData;
	QRectF inputDataRect;
	QMap<panda::BaseData*, QRectF> outputDatas;
};

#endif
