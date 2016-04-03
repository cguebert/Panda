#include <ui/drawstruct/UserValueDrawStruct.h>
#include <ui/graphview/GraphView.h>

#include <panda/PandaDocument.h>
#include <panda/object/Group.h>

#include <QPainter>

UserValueDrawStruct::UserValueDrawStruct(GraphView* view, panda::BaseGeneratorUser* object)
	: ObjectDrawStruct(view, object)
	, m_userValueObject(object)
{
	update();
}

void UserValueDrawStruct::drawText(QPainter* painter)
{
	if(m_userValueObject && !m_userValueObject->getCaption().empty())
	{
		int margin = dataRectSize+dataRectMargin+3;
		QRectF textArea = m_objectArea.adjusted(margin, 0, -margin, 0);
		QString text = QString::fromStdString(m_userValueObject->getCaption()) + "\n(" + QString::fromStdString(m_userValueObject->getName()) + ")";
		painter->drawText(textArea, Qt::AlignCenter|Qt::TextWordWrap, text);
	}
	else
		ObjectDrawStruct::drawText(painter);
}

int UserValueDrawClass = RegisterDrawObject<panda::BaseGeneratorUser, UserValueDrawStruct>();
