#ifndef USERVALUEDRAWSTRUCT_H
#define USERVALUEDRAWSTRUCT_H

#include <ui/drawstruct/ObjectDrawStruct.h>

namespace panda
{
class BaseGeneratorUser;
}

class GraphView;

class UserValueDrawStruct : public ObjectDrawStruct
{
public:
	UserValueDrawStruct(GraphView* view, panda::BaseGeneratorUser* object);

	std::string getLabel() const override;

protected:
	panda::BaseGeneratorUser* m_userValueObject;
};

#endif
