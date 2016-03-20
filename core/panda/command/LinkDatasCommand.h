#ifndef LINKDATASCOMMAND_H
#define LINKDATASCOMMAND_H

#include <panda/core.h>

#include <panda/UndoStack.h>
#include <vector>

namespace panda
{

class BaseData;
class PandaObject;

class PANDA_CORE_API LinkDatasCommand : public UndoCommand
{
public:
	LinkDatasCommand(BaseData* targetData, BaseData* parentData);

	virtual int id() const;

	virtual void redo();
	virtual void undo();

//	virtual bool mergeWith(const UndoCommand* other);

protected:
	struct LinkStruct
	{
		PandaObject	*m_targetObject,
							*m_initialParentObject,
							*m_newParentObject;
		std::string m_targetDataName,
				m_initialParentDataName,
				m_newParentDataName;
	};

	std::vector<LinkStruct> m_links;
};

} // namespace panda

#endif
