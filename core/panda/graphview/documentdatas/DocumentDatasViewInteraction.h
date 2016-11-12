#pragma once

#include <panda/graphview/ViewInteraction.h>

#include <panda/types/Rect.h>

namespace panda 
{
	
class DocumentDatas;

namespace graphview
{

	class DocumentDatasView;

	class PANDA_CORE_API DocumentDatasViewInteraction : public ViewInteraction
	{
	public:
		DocumentDatasViewInteraction(DocumentDatasView& view, DocumentDatas& documentDatas);

		void contextMenuEvent(const ContextMenuEvent& event);

		void createInputData();
		void createOutputData();
		void removeData(BaseData* data);

	protected:
		DocumentDatasView& m_documentDatasView;
		DocumentDatas& m_documentDatas;
	};

} // namespace graphview

} // namespace panda
