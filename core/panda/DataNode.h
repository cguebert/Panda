#ifndef DATANODE_H
#define DATANODE_H

#include <panda/BaseClass.h>

#include <QList>

namespace panda
{

class PANDA_CORE_API DataNode
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

	const NodesList& getInputs() const;
	const NodesList& getOutputs() const;

	virtual void update() = 0; /// Do the action required to reset the dirty flag

	bool isDirty() const;
	void doSetDirty(); /// Only change the flag, don't propagate
	void cleanDirty(); /// Reset the dirty flag

	virtual void setDirtyValue(const DataNode* caller); /// Change the flag and propagate to all outputs
	virtual void setDirtyOutputs(); /// Propagate the change to all outputs
	virtual void updateIfDirty() const;

	virtual void doAddInput(DataNode* node);
	virtual void doRemoveInput(DataNode* node);
	virtual void doAddOutput(DataNode* node);
	virtual void doRemoveOutput(DataNode* node);

protected:
	bool m_dirtyValue;
	NodesList m_inputs, m_outputs;
};

//****************************************************************************//

inline const DataNode::NodesList& DataNode::getInputs() const
{ return m_inputs; }

inline const DataNode::NodesList& DataNode::getOutputs() const
{ return m_outputs; }

inline bool DataNode::isDirty() const
{ return m_dirtyValue; }

inline void DataNode::setDirtyValue(const DataNode* /*caller*/)
{
	if(!m_dirtyValue)
	{
		doSetDirty();
		setDirtyOutputs();
	}
}

inline void DataNode::doSetDirty()
{ m_dirtyValue = true; }

inline void DataNode::setDirtyOutputs()
{
	for(DataNode* node : m_outputs)
		node->setDirtyValue(this);
}

inline void DataNode::cleanDirty()
{ m_dirtyValue = false; }

inline void DataNode::updateIfDirty() const
{
	if(isDirty())
		const_cast<DataNode*>(this)->update();
}

inline void DataNode::doAddInput(DataNode* node)
{ m_inputs.push_back(node); }

inline void DataNode::doRemoveInput(DataNode* node)
{ m_inputs.removeAll(node); }

inline void DataNode::doAddOutput(DataNode* node)
{ m_outputs.push_back(node); }

inline void DataNode::doRemoveOutput(DataNode* node)
{ m_outputs.removeAll(node); }

} // namespace panda

#endif // DATANODE_H
