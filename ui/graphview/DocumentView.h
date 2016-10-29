#pragma once

#include <ui/graphview/GraphView.h>

namespace graphview
{

class DocumentView : public GraphView
{
	Q_OBJECT

public:
	static std::unique_ptr<DocumentView> createDocumentView(panda::PandaDocument* doc, panda::ObjectsList& objectsList, MainWindow* mainWindow);

private:
	explicit DocumentView(panda::PandaDocument* doc, panda::ObjectsList& objectsList, MainWindow* mainWindow);
};

} // namespace graphview
