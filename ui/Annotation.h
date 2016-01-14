#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <panda/object/PandaObject.h>

namespace panda
{

class Annotation : public PandaObject
{
public:
	PANDA_CLASS(Annotation, PandaObject)

	Annotation(PandaDocument *doc);

	virtual void setDirtyValue(const DataNode* caller);

	Data<int> m_type;
	Data<std::string> m_text, m_font;

	enum AnnotationType { ANNOTATION_TEXT=0, ANNOTATION_ARROW, ANNOTATION_RECTANGLE, ANNOTATION_ELLIPSE };
};

} // namespace panda

#endif // ANNOTATION_H
