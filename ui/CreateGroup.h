#ifndef CREATEGROUP_H
#define CREATEGROUP_H

class GraphView;

namespace panda
{

class PandaDocument;

bool createGroup(PandaDocument* doc, GraphView* view);
bool ungroupSelection(PandaDocument* doc, GraphView* view);

} // namespace panda

#endif // CREATEGROUP_H
