#pragma once

namespace graphview
{

class GraphView;

// Modify position of selected objects
void alignHorizontallyCenter(GraphView& view);
void alignHorizontallyLeft(GraphView& view);
void alignHorizontallyRight(GraphView& view);
void alignVerticallyCenter(GraphView& view);
void alignVerticallyTop(GraphView& view);
void alignVerticallyBottom(GraphView& view);
void distributeHorizontallyCenter(GraphView& view);
void distributeHorizontallyGaps(GraphView& view);
void distributeHorizontallyLeft(GraphView& view);
void distributeHorizontallyRight(GraphView& view);
void distributeVerticallyCenter(GraphView& view);
void distributeVerticallyGaps(GraphView& view);
void distributeVerticallyTop(GraphView& view);
void distributeVerticallyBottom(GraphView& view);

} // namespace graphview
