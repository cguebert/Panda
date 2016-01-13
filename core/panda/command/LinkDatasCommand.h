#ifndef LINKDATASCOMMAND_H
#define LINKDATASCOMMAND_H

#include <panda/core.h>

#include <panda/UndoStack.h>
#include <vector>

namespace panda
{
class PandaObject;
class BaseData;
}

class PANDA_CORE_API LinkDatasCommand : public panda::UndoCommand
{
public:
	LinkDatasCommand(panda::BaseData* targetData, panda::BaseData* parentData);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

//	virtual bool mergeWith(const panda::UndoCommand* other);

protected:
	struct LinkStruct
	{
		panda::PandaObject	*m_targetObject,
							*m_initialParentObject,
							*m_newParentObject;
		std::string m_targetDataName,
				m_initialParentDataName,
				m_newParentDataName;
	};

	std::vector<LinkStruct> m_links;
};

#endif
