#pragma once

#include <ui/graphview/GraphView.h>

namespace graphview
{

class DocumentView : public GraphView
{
public:
	static std::unique_ptr<DocumentView> createDocumentView(panda::PandaDocument* doc, panda::ObjectsList& objectsList);

private:
	explicit DocumentView(panda::PandaDocument* doc, panda::ObjectsList& objectsList);
};

} // namespace graphview
