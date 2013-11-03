#ifndef DATANODE_H
#define DATANODE_H

#include <QList>
#include <panda/BaseClass.h>

namespace panda
{

class DataNode
{
public:
	PANDA_ABSTRACT_CLASS(DataNode, void)
	typedef QList<DataNode*> NodesList;

	DataNode();
	virtual ~DataNode();

	virtual void addInput(DataNode* node);
	virtual void removeInput(DataNode* node);
	virtual void addOutput(DataNode* node);
	virtual void removeOutput(DataNode* node);

	const NodesList& getInputs();
	const NodesList& getOutputs();

	virtual void update() = 0;

	virtual bool isDirty() const;
	virtual void setDirtyValue();
	virtual void setDirtyOutputs();
	virtual void cleanDirty();
	virtual void updateIfDirty() const;

	virtual void doAddInput(DataNode* node);
	virtual void doRemoveInput(DataNode* node);
	virtual void doAddOutput(DataNode* node);
	virtual void doRemoveOutput(DataNode* node);

protected:
	bool dirtyValue;
	NodesList inputs, outputs;
};

} // namespace panda

#endif // DATANODE_H
