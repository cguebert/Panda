#include <panda/graphview/DocumentView.h>

namespace panda
{

namespace graphview
{

DocumentView::DocumentView(PandaDocument* doc, ObjectsList& objectsList)
	: GraphView(doc, objectsList)
{
}

std::unique_ptr<DocumentView> DocumentView::createDocumentView(PandaDocument* doc, ObjectsList& objectsList)
{
	auto view = std::unique_ptr<DocumentView>(new DocumentView(doc, objectsList));
	view->initComponents();
	return view;
}

} // namespace graphview

} // namespace panda
