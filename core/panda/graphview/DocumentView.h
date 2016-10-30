#pragma once

#include <panda/graphview/GraphView.h>

namespace panda
{

namespace graphview
{

class PANDA_CORE_API DocumentView : public GraphView
{
public:
	static std::unique_ptr<DocumentView> createDocumentView(PandaDocument* doc, ObjectsList& objectsList);

private:
	explicit DocumentView(PandaDocument* doc, ObjectsList& objectsList);
};

} // namespace graphview

} // namespace panda
