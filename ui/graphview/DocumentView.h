#ifndef DOCUMENTVIEW_H
#define DOCUMENTVIEW_H

#include <ui/graphview/GraphView.h>

class DocumentView : public GraphView
{
	Q_OBJECT

public:
	explicit DocumentView(panda::PandaDocument* doc, panda::ObjectsList& objectsList, QWidget* parent = nullptr);

private:
	void savingObject(panda::XmlElement&, panda::PandaObject*);
	void loadingObject(const panda::XmlElement&, panda::PandaObject*);
};

#endif // DOCUMENTVIEW_H
