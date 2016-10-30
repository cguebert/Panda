#pragma once

#include <panda/types/Rect.h>

#include <map>
#include <vector>

namespace panda
{

class BaseData;

namespace graphview
{

namespace graphics {
	struct DrawColors;
	class DrawList;
}

class GraphView;

// NB: in a link, the input is connected to an output Data of an object, and the output to an input Data
class PANDA_CORE_API LinkTag
{
public:
	LinkTag(GraphView& view, BaseData* input, BaseData* output, int index);
	void addOutput(BaseData* output);
	void removeOutput(BaseData* output);
	void update();
	bool isEmpty();
	std::pair<BaseData*, types::Rect> getDataAtPoint(const types::Point& point);

	void moveView(const types::Point& delta);
	void draw(graphics::DrawList& list, graphics::DrawColors& colors);

	BaseData* getInputData() const;
	std::vector<BaseData*> getOutputDatas() const;

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

	BaseData* m_inputData = nullptr;

	using DataRects = std::pair<types::Rect, types::Rect>; // Tag rect, Data rect
	DataRects m_inputDataRects;
	std::map<BaseData*, DataRects> m_outputDatas;
};

inline BaseData* LinkTag::getInputData() const
{ return m_inputData; }

inline int LinkTag::index() const
{ return m_index; }

inline bool LinkTag::isHovering() const
{ return m_hovering; }

inline void LinkTag::setHovering(bool h)
{ m_hovering = h; }

} // namespace graphview

} // namespace panda
