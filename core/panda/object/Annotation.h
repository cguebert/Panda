#ifndef ANNOTATION_H
#define ANNOTATION_H

#include <panda/object/PandaObject.h>
#include <panda/types/Color.h>
#include <panda/types/Point.h>
#include <panda/messaging.h>

namespace panda
{

class PANDA_CORE_API Annotation : public PandaObject
{
public:
	PANDA_CLASS(Annotation, PandaObject)

	Annotation(PandaDocument *doc);

	virtual void setDirtyValue(const DataNode* caller);

	Data<int> m_type;
	Data<std::string> m_text;
	Data<types::Color> m_color;

	enum AnnotationType { ANNOTATION_TEXT=0, ANNOTATION_ARROW, ANNOTATION_RECTANGLE, ANNOTATION_ELLIPSE };

	types::Point getDeltaToEnd();
	void setDeltaToEnd(types::Point delta);

	panda::msg::Signal<void()> deltaToEndChanged;

private:
	Data<types::Point> m_deltaToEnd;
};

} // namespace panda

#endif // ANNOTATION_H
