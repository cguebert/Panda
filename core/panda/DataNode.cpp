#include <panda/DataNode.h>

#include <algorithm>

namespace panda
{

DataNode::DataNode()
	: m_dirtyValue(false)
{
}

DataNode::~DataNode()
{
	for(DataNode* node : m_inputs)
		node->doRemoveOutput(*this);
	for(DataNode* node : m_outputs)
		node->doRemoveInput(*this);
}

void DataNode::addInput(DataNode& node)
{
	doAddInput(node);
	node.doAddOutput(*this);
	setDirtyValue(&node);
}

void DataNode::removeInput(DataNode& node)
{
	doRemoveInput(node);
	node.doRemoveOutput(*this);
}

void DataNode::addOutput(DataNode& node)
{
	doAddOutput(node);
	node.doAddInput(*this);
	node.setDirtyValue(this);
}

void DataNode::removeOutput(DataNode& node)
{
	doRemoveOutput(node);
	node.doRemoveInput(*this);
}

void DataNode::doRemoveInput(DataNode& node)
{ 
	m_inputs.erase(std::remove(m_inputs.begin(), m_inputs.end(), &node), m_inputs.end()); 
}

void DataNode::doRemoveOutput(DataNode& node)
{	
	m_outputs.erase(std::remove(m_outputs.begin(), m_outputs.end(), &node), m_outputs.end()); 
}

} // namespace panda
