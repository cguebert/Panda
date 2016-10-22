#pragma once

#include <ui/graphview/GraphView.h>

namespace graphview
{

class DocumentView : public GraphView
{
	Q_OBJECT

public:
	explicit DocumentView(panda::PandaDocument* doc, panda::ObjectsList& objectsList, QWidget* parent = nullptr);
};

} // namespace graphview
