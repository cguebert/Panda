#include <ui/graphview/DocumentView.h>

DocumentView::DocumentView(panda::PandaDocument* doc, panda::ObjectsList& objectsList, QWidget* parent)
	: GraphView(doc, objectsList, parent)
{
}