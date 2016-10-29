#include <ui/graphview/DocumentView.h>

namespace graphview
{

DocumentView::DocumentView(panda::PandaDocument* doc, panda::ObjectsList& objectsList, MainWindow* mainWindow)
	: GraphView(doc, objectsList, mainWindow)
{
}

std::unique_ptr<DocumentView> DocumentView::createDocumentView(panda::PandaDocument* doc, panda::ObjectsList& objectsList, MainWindow* mainWindow)
{
	auto view = std::unique_ptr<DocumentView>(new DocumentView(doc, objectsList, mainWindow));
	view->initComponents();
	return view;
}

} // namespace graphview
