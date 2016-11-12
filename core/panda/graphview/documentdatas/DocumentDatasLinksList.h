#pragma once

#include <panda/graphview/LinksList.h>

#include <panda/types/Rect.h>

namespace panda 
{
	
class BaseData;
class DocumentDatas;

namespace graphview
{
	class DocumentDatasView;

	class PANDA_CORE_API DocumentDatasLinksList : public LinksList
	{
	public:
		DocumentDatasLinksList(DocumentDatasView& view, DocumentDatas& documentDatas);

		bool getDataRect(const BaseData* data, types::Rect& rect) override;
		DataRect getDataAtPos(const types::Point& pt) override;

		ConnectedDatas getConnectedDatas(BaseData* srcData) override;
		
		bool createLink(BaseData* data1, BaseData* data2) override;
		bool isCompatible(const BaseData* data1, const BaseData* data2) override;
		void computeCompatibleDatas(BaseData* data) override;

	protected:
		void updateLinks(const graphics::DrawColors& colors) override;

		DocumentDatasView& m_documentDatasView;
		DocumentDatas& m_documentDatas;
	};

} // namespace graphview

} // namespace panda
