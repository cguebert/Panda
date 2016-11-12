#pragma once

#include <panda/graphview/Viewport.h>

#include <panda/types/Rect.h>

namespace panda 
{
	
class DocumentDatas;

namespace graphview
{
	
	class PANDA_CORE_API DocumentDatasViewport : public Viewport
	{
	public:
		DocumentDatasViewport(GraphView& view, DocumentDatas& documentDatas);

		types::Rect onlyObjectsRect() const;

		void updateObjectsRect() override;

	private:
		DocumentDatas& m_documentDatas;
		types::Rect m_onlyObjectsRect; // Without the document datas
	};

//****************************************************************************//

inline types::Rect DocumentDatasViewport::onlyObjectsRect() const
{ return m_onlyObjectsRect; }

} // namespace graphview

} // namespace panda
