#pragma once

#include <panda/core.h>

namespace panda
{

namespace graphview {
	class GraphView;
}

class PandaDocument;

bool PANDA_CORE_API createGroup(PandaDocument* doc, graphview::GraphView* view);
bool PANDA_CORE_API ungroupSelection(PandaDocument* doc, graphview::GraphView* view);

} // namespace panda

