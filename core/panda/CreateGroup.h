#pragma once

#include <panda/core.h>

namespace graphview {
	class GraphView;
}

namespace panda
{

class PandaDocument;

bool PANDA_CORE_API createGroup(PandaDocument* doc, graphview::GraphView* view);
bool PANDA_CORE_API ungroupSelection(PandaDocument* doc, graphview::GraphView* view);

} // namespace panda

