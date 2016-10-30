#pragma once

#include <panda/graphview/GraphView.h>

namespace graphview
{

class PANDA_CORE_API DocumentView : public GraphView
{
public:
	static std::unique_ptr<DocumentView> createDocumentView(panda::PandaDocument* doc, panda::ObjectsList& objectsList);

private:
	explicit DocumentView(panda::PandaDocument* doc, panda::ObjectsList& objectsList);
};

} // namespace graphview
