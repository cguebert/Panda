#pragma once

#include <panda/types/Rect.h>

#include <panda/graphview/graphics/DrawList.h>

#include <memory>
#include <set>

namespace panda
{

class BaseData;

namespace graphview
{

	namespace graphics {
		struct DrawColors;
	}

	class GraphView;

	class PANDA_CORE_API LinksList
	{
	public:
		using DataRect = std::pair<BaseData*, types::Rect>;
		using Rects = std::vector<types::Rect>;
		using PointsPairs = std::vector<std::pair<types::Point, types::Point>>;
		using ConnectedDatas = std::pair<Rects, PointsPairs>;

		LinksList(GraphView& view);
		virtual ~LinksList();

		void clear();

		void initializeRenderer(ViewRenderer& viewRenderer);
		void onBeginDraw(const graphics::DrawColors& colors);
		const std::shared_ptr<graphics::DrawList>& linksDrawList() const;

		virtual bool getDataRect(const BaseData* data, types::Rect& rect);
		virtual DataRect getDataAtPos(const types::Point& pt);
		virtual ConnectedDatas getConnectedDatas(BaseData* data);
		
		virtual bool createLink(BaseData* data1, BaseData* data2);
		virtual bool isCompatible(const BaseData* data1, const BaseData* data2);
		virtual void computeCompatibleDatas(BaseData* data);

		bool canLinkWith(const BaseData* data) const; // Is it possible to link this data and the clicked data
		void changeLink(BaseData* target, BaseData* parent);

	protected:
		virtual void updateLinks(const graphics::DrawColors& colors);

		GraphView& m_view;

		std::shared_ptr<graphics::DrawList> m_linksDrawList;
		bool m_recomputeLinks = true;

		std::set<const BaseData*> m_possibleLinks; // When creating a new link, this contains all possible destinations
	};

//****************************************************************************//

	inline const std::shared_ptr<graphics::DrawList>& LinksList::linksDrawList() const
	{ return m_linksDrawList; }

	inline bool LinksList::canLinkWith(const BaseData* data) const
	{ return m_possibleLinks.count(data) != 0; }

} // namespace graphview

} // namespace panda
