#ifndef GENERICOBJECTDRAWSTRUCT_H
#define GENERICOBJECTDRAWSTRUCT_H

#include <ui/drawstruct/ObjectDrawStruct.h>

namespace panda
{
class GenericObject;
}

class GenericObjectDrawStruct : public ObjectDrawStruct
{
public:
	GenericObjectDrawStruct(GraphView* view, panda::GenericObject* m_object);

	virtual void update();
	virtual void drawDatas(QPainter* painter);
	virtual QSize getObjectSize();

	virtual QColor getDataColor(const panda::BaseData* data);

	static const int createdDataRectMargin = 3;

protected:
	panda::GenericObject* m_genericObject;
	int m_nbDefInputs, m_nbDefOutputs;
};

#endif
