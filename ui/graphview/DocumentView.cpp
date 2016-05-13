#include <ui/drawstruct/ObjectDrawStruct.h>
#include <ui/drawstruct/DockableDrawStruct.h>
#include <ui/graphview/DocumentView.h>
#include <ui/graphview/LinkTag.h>
#include <ui/graphview/ObjectsSelection.h>
#include <ui/graphview/ViewRenderer.h>
#include <ui/graphview/graphics/DrawList.h>

#include <panda/PandaDocument.h>
#include <panda/XmlDocument.h>
#include <panda/document/DocumentSignals.h>
#include <panda/document/ObjectsList.h>

using Point = panda::types::Point;
using Rect = panda::types::Rect;

DocumentView::DocumentView(panda::PandaDocument* doc, panda::ObjectsList& objectsList, QWidget* parent)
	: GraphView(doc, objectsList, parent)
{
	auto& docSignals = m_pandaDocument->getSignals();
	m_observer.get(docSignals.savingObject).connect<DocumentView, &DocumentView::savingObject>(this);
	m_observer.get(docSignals.loadingObject).connect<DocumentView, &DocumentView::loadingObject>(this);
}

void DocumentView::savingObject(panda::XmlElement& elem, panda::PandaObject* object)
{
	getObjectDrawStruct(object)->save(elem);

	// Save data labels for this object
	for (const auto& dl : m_dataLabels)
	{
		if (dl.object == object)
		{
			auto dlNode = elem.addChild("DataLabel");
			dlNode.setAttribute("data", dl.data->getName());
			dlNode.setText(dl.label);
		}
	}
}

void DocumentView::loadingObject(const panda::XmlElement& elem, panda::PandaObject* object)
{
	getObjectDrawStruct(object)->load(elem);

	// Load data labels
	auto e = elem.firstChild("DataLabel");
	while(e)
	{
		auto data = object->getData(e.attribute("data").toString());
		if (data)
		{
			DataLabel dl;
			dl.data = data;
			dl.object = object;
			dl.label = e.text();
			m_dataLabels.push_back(dl);
		}
		e = e.nextSibling("DataLabel");
	}
}
