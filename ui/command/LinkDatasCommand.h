#ifndef LINKDATASCOMMAND_H
#define LINKDATASCOMMAND_H

#include <QUndoCommand>
#include <QVector>

namespace panda
{
class PandaObject;
class BaseData;
}

class LinkDatasCommand : public QUndoCommand
{
public:
	LinkDatasCommand(panda::BaseData* targetData, panda::BaseData* parentData, QUndoCommand* parent = nullptr);

	virtual int id() const { return 3; }

	virtual void redo();
	virtual void undo();

//	virtual bool mergeWith(const QUndoCommand *other);

protected:
	panda::PandaDocument* m_document;

	struct LinkStruct
	{
		panda::PandaObject	*m_targetObject,
							*m_initialParentObject,
							*m_newParentObject;
		QString m_targetDataName,
				m_initialParentDataName,
				m_newParentDataName;
	};

	QVector<LinkStruct> m_links;
};

#endif
