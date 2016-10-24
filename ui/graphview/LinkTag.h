#pragma once

#include <panda/types/Rect.h>
#include <map>

namespace panda
{
class BaseData;
}

namespace graphview
{

namespace graphics {
	struct DrawColors;
	class DrawList;
}

class GraphView;

// NB: in a link, the input is connected to an output Data of an object, and the output to an input Data
class LinkTag
{
public:
	LinkTag(GraphView& view, panda::BaseData* input, panda::BaseData* output, int index);
	void addOutput(panda::BaseData* output);
	void removeOutput(panda::BaseData* output);
	void update();
	bool isEmpty();
	std::pair<panda::BaseData*, panda::types::Rect> getDataAtPoint(const panda::types::Point& point);

	void moveView(const panda::types::Point& delta);
	void draw(graphics::DrawList& list, graphics::DrawColors& colors);

	panda::BaseData* getInputData() const;
	std::vector<panda::BaseData*> getOutputDatas() const;

	int index() const;
	bool isHovering() const;
	void setHovering(bool h);

	static bool needLinkTag(float inputX, float outputX, GraphView& view);

private:
	static const int tagW = 20;
	static const int tagH = 13;
	static const int tagMargin = 10;

	const int m_index = 0;
	bool m_hovering = false;

	GraphView& m_parentView;

	panda::BaseData* m_inputData = nullptr;

	using DataRects = std::pair<panda::types::Rect, panda::types::Rect>; // Tag rect, Data rect
	DataRects m_inputDataRects;
	std::map<panda::BaseData*, DataRects> m_outputDatas;
};

inline panda::BaseData* LinkTag::getInputData() const
{ return m_inputData; }

inline int LinkTag::index() const
{ return m_index; }

inline bool LinkTag::isHovering() const
{ return m_hovering; }

inline void LinkTag::setHovering(bool h)
{ m_hovering = h; }

} // namespace graphview
