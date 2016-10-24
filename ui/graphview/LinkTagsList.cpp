#include <ui/graphview/LinkTagsList.h>
#include <ui/graphview/GraphView.h>
#include <ui/graphview/ObjectRenderersList.h>
#include <ui/graphview/object/ObjectRenderer.h>

#include <panda/object/PandaObject.h>
#include <panda/document/ObjectsList.h>

#include <panda/helper/algorithm.h>

using panda::types::Point;
using panda::types::Rect;

namespace graphview
{

	LinkTagsList::LinkTagsList(GraphView& view)
		: m_view(view)
	{
	}

	void LinkTagsList::addLinkTag(panda::BaseData* input, panda::BaseData* output)
	{
		if(m_linkTagsMap.count(input))
			m_linkTagsMap[input]->addOutput(output);
		else
		{
			auto tag = std::make_shared<LinkTag>(m_view, input, output, getAvailableLinkTagIndex());
			m_linkTags.push_back(tag);
			m_linkTagsMap[input] = tag.get();
		}
	}

	void LinkTagsList::removeLinkTag(panda::BaseData* input, panda::BaseData* output)
	{
		if(m_linkTagsMap.count(input))
		{
			LinkTag* tag = m_linkTagsMap[input];
			tag->removeOutput(output);
			if (tag->isEmpty())
			{
				m_linkTagsMap.erase(input);
				panda::helper::removeIf(m_linkTags, [tag](const auto& ptr) {
					return ptr.get() == tag;
				});
			}
		}
	}

	bool LinkTagsList::hasLinkTag(panda::BaseData* input, panda::BaseData* output)
	{
		return m_linkTagsDatas.count(std::make_pair(input, output)) != 0;
	}

	void LinkTagsList::onBeginDraw()
	{
		if(m_recomputeTags)	
			updateLinkTags();
	}

	void LinkTagsList::drawTags(graphics::DrawList& drawList, graphics::DrawColors& colors)
	{
		// Draw links tags
		for (auto& tag : m_linkTags)
			tag->draw(drawList, colors);
	}

	void LinkTagsList::updateLinkTags()
	{
		m_recomputeTags = false;
		// Testing all links and adding new tags
		for(auto& object : m_view.objectsList().get())
		{
			for(auto& data : object->getInputDatas())
			{
				panda::BaseData* parentData = data->getParent();
				if(parentData)
				{
					auto ownerObjRnd = m_view.objectRenderers().get(data->getOwner());
					auto parentObjRnd = m_view.objectRenderers().get(parentData->getOwner());
					if (ownerObjRnd && parentObjRnd)
					{
						Rect ownerRect, parentRect;
						ownerObjRnd->getDataRect(data, ownerRect);
						parentObjRnd->getDataRect(parentData, parentRect);
						float ox = ownerRect.center().x;
						float ix = parentRect.center().x;
						if (LinkTag::needLinkTag(ix, ox, m_view))
							addLinkTag(parentData, data);
					}
				}
			}
		}

		// Updating tags
		for (auto& linkTag : m_linkTags)
			linkTag->update();

		// Removing empty ones from the map
		for (auto it = m_linkTagsMap.begin(); it != m_linkTagsMap.end();)
		{
			if (it->second->isEmpty())
				it = m_linkTagsMap.erase(it);
			else
				++it;
		}

		// And removing them from the list (freeing them in the process)
		auto last = std::remove_if(m_linkTags.begin(), m_linkTags.end(), [](const auto& tag) {
			return tag->isEmpty();
		});
		m_linkTags.erase(last, m_linkTags.end());

		// Updating the connections list
		m_linkTagsDatas.clear();
		for (const auto& linkTag : m_linkTags)
		{
			const auto input = linkTag->getInputData();
			for (const auto output : linkTag->getOutputDatas())
				m_linkTagsDatas.emplace(input, output);
		}
	}

	void LinkTagsList::clear()
	{
		m_linkTags.clear();
		m_linkTagsMap.clear();
		m_recomputeTags = true;
	}

	void LinkTagsList::setDirty()
	{
		m_recomputeTags = true;
	}

	int LinkTagsList::getAvailableLinkTagIndex()
	{
		int nb = m_linkTags.size();
		std::vector<bool> indices(nb, true);

		for(const auto& linkTag : m_linkTags)
		{
			int id = linkTag->index();
			if(id < nb)
				indices[id] = false;
		}

		for(int i=0; i<nb; ++i)
		{
			if(indices[i])
				return i;
		}

		return nb;
	}

} // namespace graphview
