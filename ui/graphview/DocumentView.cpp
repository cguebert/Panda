#include <ui/graphview/DocumentView.h>

namespace graphview
{

DocumentView::DocumentView(panda::PandaDocument* doc, panda::ObjectsList& objectsList, MainWindow* mainWindow)
	: GraphView(doc, objectsList, mainWindow)
{
}

} // namespace graphview
