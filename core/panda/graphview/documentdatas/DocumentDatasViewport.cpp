#include <panda/graphview/documentdatas/DocumentDatasViewport.h>

#include <panda/graphview/GraphView.h>
#include <panda/graphview/ObjectRenderersList.h>
#include <panda/graphview/object/ObjectRenderer.h>
#include <panda/document/DocumentDatas.h>

namespace
{
	static const int dataRectSize = 10;
	static const int dataMarginW = 100;
	static const float tagW = 18;
	static const float tagMargin = 10;
}

namespace panda
{

namespace graphview
{

	DocumentDatasViewport::DocumentDatasViewport(GraphView& view, DocumentDatas& documentDatas)
		: Viewport(view)
		, m_documentDatas(documentDatas)
	{
	}

	void DocumentDatasViewport::updateObjectsRect()
	{
		if(m_view.isLoading())
			return;

		m_objectsRect = {};
		for(const auto& objRnd : m_view.objectRenderers().getOrdered())
			m_objectsRect |= objRnd->getVisualArea();
		m_onlyObjectsRect = m_objectsRect;

		int nbInputs = 0, nbOutputs = 0;
		for (const auto& data : m_documentDatas.get())
		{
			if (data->isInput())	++nbInputs;
			if (data->isOutput())	++nbOutputs;
		}

		if (nbInputs)
			m_objectsRect.adjust(-(dataMarginW + dataRectSize + tagMargin + tagW), 0, 0, 0);
		if (nbOutputs)
			m_objectsRect.adjust(0, 0, dataMarginW + tagMargin + tagW, 0);

		// Ensure the area is not empty if there is at least one data
		if ((nbInputs || nbOutputs) && !m_objectsRect.height())
			m_objectsRect.setHeight(10.f); 

		updateViewRect();
	}

} // namespace graphview

} // namespace panda
