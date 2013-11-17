#ifndef GENERICOBJECTDRAWSTRUCT_H
#define GENERICOBJECTDRAWSTRUCT_H

#include <ui/ObjectDrawStruct.h>

namespace panda
{
class GenericObject;
}

class GenericObjectDrawStruct : public ObjectDrawStruct
{
public:
	GenericObjectDrawStruct(GraphView* view, panda::GenericObject* object);

	virtual void update();
	virtual void drawDatas(QPainter* painter);
	virtual QSize getObjectSize();

	static const int createdDataRectMargin = 3;

protected:
	panda::GenericObject* genericObject;
	int nbDefInputs, nbDefOutputs;
};

#endif
