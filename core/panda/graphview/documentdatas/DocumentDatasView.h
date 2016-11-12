#pragma once

#include <panda/graphview/GraphView.h>

#include <panda/types/Rect.h>

namespace panda 
{
	
class BaseData;
class DocumentDatas;

namespace graphview
{

class PANDA_CORE_API DocumentDatasView : public GraphView
{
public:
	using DataRect = std::pair<BaseData*, types::Rect>;
	using DataRects = std::vector<DataRect>;

	static std::unique_ptr<DocumentDatasView> createDocumentDatasView(DocumentDatas& documentDatas, PandaDocument* doc, ObjectsList& objectsList);

	void updateDataRects();
	const DataRects& dataRects() const;

protected:
	void initializeRenderer(ViewRenderer& viewRenderer) override;
	void drawGraphView(ViewRenderer& viewRenderer, graphics::DrawColors drawColors) override;

private:
	DocumentDatasView(DocumentDatas& documentDatas, PandaDocument* doc, ObjectsList& objectsList);

	void modifiedObject(PandaObject* object);

	DocumentDatas& m_documentDatas;
	DataRects m_dataRects;
	std::shared_ptr<graphics::DrawList> m_documentDatasDrawList;
};

//****************************************************************************//

inline const DocumentDatasView::DataRects& DocumentDatasView::dataRects() const
{ return m_dataRects; }

} // namespace graphview

} // namespace panda
