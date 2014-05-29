#include <ui/drawstruct/UserValueDrawStruct.h>
#include <ui/GraphView.h>

#include <panda/PandaDocument.h>
#include <modules/generators/UserValue.h>

#include <QPainter>

UserValueDrawStruct::UserValueDrawStruct(GraphView* view, panda::BaseGeneratorUser* object)
	: ObjectDrawStruct(view, object)
	, m_userValueObject(object)
{
	update();
}

void UserValueDrawStruct::drawText(QPainter* painter)
{
	if(m_userValueObject && !m_userValueObject->getCaption().isEmpty())
	{
		int margin = dataRectSize+dataRectMargin+3;
		QRectF textArea = m_objectArea.adjusted(margin, 0, -margin, 0);
		QString text = m_userValueObject->getCaption() + "\n(" + m_userValueObject->getName() + ")";
		painter->drawText(textArea, Qt::AlignCenter|Qt::TextWordWrap, text);
	}
	else
		ObjectDrawStruct::drawText(painter);
}

int UserValueDrawClass = RegisterDrawObject<panda::BaseGeneratorUser, UserValueDrawStruct>();