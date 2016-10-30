#pragma once

#include <panda/core.h>

namespace panda
{

namespace graphview
{

class GraphView;

// Modify position of selected objects
void PANDA_CORE_API alignHorizontallyCenter(GraphView& view);
void PANDA_CORE_API alignHorizontallyLeft(GraphView& view);
void PANDA_CORE_API alignHorizontallyRight(GraphView& view);
void PANDA_CORE_API alignVerticallyCenter(GraphView& view);
void PANDA_CORE_API alignVerticallyTop(GraphView& view);
void PANDA_CORE_API alignVerticallyBottom(GraphView& view);
void PANDA_CORE_API distributeHorizontallyCenter(GraphView& view);
void PANDA_CORE_API distributeHorizontallyGaps(GraphView& view);
void PANDA_CORE_API distributeHorizontallyLeft(GraphView& view);
void PANDA_CORE_API distributeHorizontallyRight(GraphView& view);
void PANDA_CORE_API distributeVerticallyCenter(GraphView& view);
void PANDA_CORE_API distributeVerticallyGaps(GraphView& view);
void PANDA_CORE_API distributeVerticallyTop(GraphView& view);
void PANDA_CORE_API distributeVerticallyBottom(GraphView& view);

} // namespace graphview

} // namespace panda
