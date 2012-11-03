#ifndef GENERICOBJECTDRAWSTRUCT_H
#define GENERICOBJECTDRAWSTRUCT_H

#include <panda/GenericObject.h>

#include <ui/ObjectDrawStruct.h>

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
