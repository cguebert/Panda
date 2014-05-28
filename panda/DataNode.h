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
	virtual void setDirtyValue(const DataNode* caller);
	virtual void doSetDirty();
	virtual void setDirtyOutputs();
	virtual void cleanDirty();
	virtual void updateIfDirty() const;

	virtual void doAddInput(DataNode* node);
	virtual void doRemoveInput(DataNode* node);
	virtual void doAddOutput(DataNode* node);
	virtual void doRemoveOutput(DataNode* node);

protected:
	bool m_dirtyValue;
	NodesList m_inputs, m_outputs;
};

//***************************************************************//

inline const DataNode::NodesList& DataNode::getInputs()
{ return m_inputs; }

inline const DataNode::NodesList& DataNode::getOutputs()
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
{ m_inputs.append(node); }

inline void DataNode::doRemoveInput(DataNode* node)
{ m_inputs.removeAll(node); }

inline void DataNode::doAddOutput(DataNode* node)
{ m_outputs.append(node); }

inline void DataNode::doRemoveOutput(DataNode* node)
{ m_outputs.removeAll(node); }

} // namespace panda

#endif // DATANODE_H
