#pragma once

#include <ui/graphview/LinkTag.h>

#include <memory>
#include <set>

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
	class LinkTagsList
	{
	public:
		LinkTagsList(GraphView& view);

		void addLinkTag(panda::BaseData* input, panda::BaseData* output);
		void removeLinkTag(panda::BaseData* input, panda::BaseData* output);
		bool hasLinkTag(panda::BaseData* input, panda::BaseData* output);

		void onBeginDraw();
		void onEndDraw(graphics::DrawList& list, graphics::DrawColors& colors);

		void clear();
		void setDirty();

	protected:
		int getAvailableLinkTagIndex();

		void updateLinkTags();

		GraphView& m_view;

		bool m_recomputeTags = false; // Should we recompute the linkTags next PaintEvent?
		std::vector<std::shared_ptr<LinkTag>> m_linkTags;
		std::map<panda::BaseData*, LinkTag*> m_linkTagsMap; // Input data of the link tag
		std::set<std::pair<panda::BaseData*, panda::BaseData*>> m_linkTagsDatas; // A copy of the link tags connections
	};

} // namespace graphview
