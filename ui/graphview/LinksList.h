#pragma once

#include <panda/types/Rect.h>

#include <ui/graphview/graphics/DrawList.h>

#include <memory>
#include <set>

namespace panda {
	class BaseData;
}

namespace graphview
{

	namespace graphics {
		struct DrawColors;
	}

	class GraphView;

	class LinksList
	{
	public:
		using DataRect = std::pair<panda::BaseData*, panda::types::Rect>;
		using Rects = std::vector<panda::types::Rect>;
		using PointsPairs = std::vector<std::pair<panda::types::Point, panda::types::Point>>;
		using ConnectedDatas = std::pair<Rects, PointsPairs>;

		LinksList(GraphView& view);
		virtual ~LinksList();

		void clear();

		void initializeRenderer(ViewRenderer& viewRenderer);
		void onBeginDraw(const graphics::DrawColors& colors);
		const std::shared_ptr<graphics::DrawList>& linksDrawList() const;

		virtual bool getDataRect(const panda::BaseData* data, panda::types::Rect& rect);
		virtual DataRect getDataAtPos(const panda::types::Point& pt);
		virtual ConnectedDatas getConnectedDatas(panda::BaseData* data);
		
		virtual bool createLink(panda::BaseData* data1, panda::BaseData* data2);
		virtual bool isCompatible(const panda::BaseData* data1, const panda::BaseData* data2);
		virtual void computeCompatibleDatas(panda::BaseData* data);

		bool canLinkWith(const panda::BaseData* data) const; // Is it possible to link this data and the clicked data
		void changeLink(panda::BaseData* target, panda::BaseData* parent);

	protected:
		virtual void updateLinks(const graphics::DrawColors& colors);

		GraphView& m_view;

		std::shared_ptr<graphics::DrawList> m_linksDrawList;
		bool m_recomputeLinks = true;

		std::set<const panda::BaseData*> m_possibleLinks; // When creating a new link, this contains all possible destinations
	};

//****************************************************************************//

	inline const std::shared_ptr<graphics::DrawList>& LinksList::linksDrawList() const
	{ return m_linksDrawList; }

	inline bool LinksList::canLinkWith(const panda::BaseData* data) const
	{ return m_possibleLinks.count(data) != 0; }

} // namespace graphview
