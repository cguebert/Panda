#ifndef LINKTAG_H
#define LINKTAG_H

#include <QRectF>
#include <map>

namespace panda
{
class BaseData;
}

class GraphView;

// NB: in a link, the input is connected to an output Data of an object, and the output to an input Data
class LinkTag
{
public:
	LinkTag(GraphView* view, panda::BaseData* input, panda::BaseData* output, int index);
	void addOutput(panda::BaseData* output);
	void removeOutput(panda::BaseData* output);
	void update();
	bool isEmpty();
	bool containsPoint(const QPointF& point);

	void moveView(const QPointF& delta);
	void draw(QPainter* painter);

	panda::BaseData* getInputData() const;
	std::vector<panda::BaseData*> getOutputDatas() const;

	int index() const;
	bool isHovering() const;
	void setHovering(bool h);

	static bool needLinkTag(float inputX, float outputX);

private:
	static const int tagW = 20;
	static const int tagH = 11;
	static const int tagMargin = 10;

	const int m_index = 0;
	bool m_hovering = false;

	GraphView* m_parentView = nullptr;
	panda::BaseData* m_inputData = nullptr;
	QRectF m_inputDataRect;
	std::map<panda::BaseData*, QRectF> m_outputDatas;
};

inline panda::BaseData* LinkTag::getInputData() const
{ return m_inputData; }

inline int LinkTag::index() const
{ return m_index; }

inline bool LinkTag::isHovering() const
{ return m_hovering; }

inline void LinkTag::setHovering(bool h)
{ m_hovering = h; }

#endif
