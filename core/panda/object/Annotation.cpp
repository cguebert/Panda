#include <panda/PandaDocument.h>
#include <panda/object/ObjectFactory.h>

#include <panda/object/Annotation.h>

namespace panda
{

Annotation::Annotation(PandaDocument *doc)
	: PandaObject(doc)
	, m_type(initData(0, "type", "Type of the annotation"))
	, m_text(initData(std::string("type text here"), "text", "Text of the annotation"))
	, m_font(initData("font", "Font of the annotation"))
	, m_deltaToEnd(initData(types::Point(200, 100), "deltaToEnd", "Position of the end of the shape"))
{
	addInput(m_type);
	addInput(m_text);
	addInput(m_font);
	addInput(m_deltaToEnd);

	m_type.setWidget("enum");
	m_type.setWidgetData("Text only;Arrow;Rectangle;Ellipse");
	m_text.setWidget("multiline");
	m_font.setWidget("font");

	m_deltaToEnd.setDisplayed(false);
}

void Annotation::setDirtyValue(const DataNode* caller)
{
	if(!isDirty())
	{
		PandaObject::setDirtyValue(caller);

		emitModified();
	}
	else
		PandaObject::setDirtyValue(caller);
}

int AnnotationClass = RegisterObject<Annotation>("Annotation").setDescription("Create an annotation in the graph view");

} // namespace panda
