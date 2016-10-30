#include <panda/graphview/DocumentView.h>

namespace graphview
{

DocumentView::DocumentView(panda::PandaDocument* doc, panda::ObjectsList& objectsList)
	: GraphView(doc, objectsList)
{
}

std::unique_ptr<DocumentView> DocumentView::createDocumentView(panda::PandaDocument* doc, panda::ObjectsList& objectsList)
{
	auto view = std::unique_ptr<DocumentView>(new DocumentView(doc, objectsList));
	view->initComponents();
	return view;
}

} // namespace graphview
