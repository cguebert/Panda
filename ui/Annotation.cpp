#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>

#include <ui/Annotation.h>

namespace panda
{

Annotation::Annotation(PandaDocument *doc)
	: PandaObject(doc)
	, m_type(initData(&m_type, 0, "type", "Type of the annotation"))
	, m_text(initData(&m_text, QString("type text here"), "text", "Text of the annotation"))
	, m_font(initData(&m_font, "font", "Font of the annotation"))
{
	addInput(&m_type);
	addInput(&m_text);
	addInput(&m_font);

	m_type.setWidget("enum");
	m_type.setWidgetData("Text only;Arrow;Rectangle;Ellipse");
	m_text.setWidget("multiline");
	m_font.setWidget("font");
}

void Annotation::setDirtyValue()
{
	if(!isDirty())
	{
		PandaObject::setDirtyValue();

		emitModified();
	}
	else
		PandaObject::setDirtyValue();
}

int AnnotationClass = RegisterObject<Annotation>("Annotation").setDescription("Create an annotation in the graph view");

} // namespace panda
