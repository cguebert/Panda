#include <ui/graphview/DocumentView.h>

namespace graphview
{

DocumentView::DocumentView(panda::PandaDocument* doc, panda::ObjectsList& objectsList, QWidget* parent)
	: GraphView(doc, objectsList, parent)
{
}

} // namespace graphview
