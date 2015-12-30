#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>

#include <ui/Annotation.h>

namespace panda
{

Annotation::Annotation(PandaDocument *doc)
	: PandaObject(doc)
	, m_type(initData(0, "type", "Type of the annotation"))
	, m_text(initData(std::string("type text here"), "text", "Text of the annotation"))
	, m_font(initData("font", "Font of the annotation"))
{
	addInput(m_type);
	addInput(m_text);
	addInput(m_font);

	m_type.setWidget("enum");
	m_type.setWidgetData("Text only;Arrow;Rectangle;Ellipse");
	m_text.setWidget("multiline");
	m_font.setWidget("font");
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
