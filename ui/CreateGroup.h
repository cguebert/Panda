#pragma once

namespace graphview {
	class GraphView;
}

namespace panda
{

class PandaDocument;

bool createGroup(PandaDocument* doc, graphview::GraphView* view);
bool ungroupSelection(PandaDocument* doc, graphview::GraphView* view);

} // namespace panda

