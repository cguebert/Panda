#include <panda/DataNode.h>

namespace panda
{

DataNode::DataNode()
    : dirtyValue(false)
{
}

DataNode::~DataNode()
{
    foreach(DataNode* node, inputs)
        node->removeOutput(this);
		//node->doRemoveOutput(this);
    foreach(DataNode* node, outputs)
        node->removeInput(this);
		//node->doRemoveInput(this);
}

void DataNode::addInput(DataNode* node)
{
    doAddInput(node);
    node->doAddOutput(this);
    setDirtyValue();
}

void DataNode::removeInput(DataNode* node)
{
    doRemoveInput(node);
    node->doRemoveOutput(this);
}

void DataNode::addOutput(DataNode* node)
{
    doAddOutput(node);
    node->doAddInput(this);
    node->setDirtyValue();
}

void DataNode::removeOutput(DataNode* node)
{
    doRemoveOutput(node);
    node->doRemoveInput(this);
}

const DataNode::NodesList& DataNode::getInputs()
{
    return inputs;
}

const DataNode::NodesList& DataNode::getOutputs()
{
    return outputs;
}

bool DataNode::isDirty() const
{
    return dirtyValue;
}

void DataNode::setDirtyValue()
{
    if(!dirtyValue)
    {
        dirtyValue = true;
        setDirtyOutputs();
    }
}

void DataNode::setDirtyOutputs()
{
	foreach(DataNode* node, outputs)
		node->setDirtyValue();
}

void DataNode::cleanDirty()
{
	dirtyValue = false;
}

void DataNode::updateIfDirty() const
{
    if(isDirty())
        const_cast<DataNode*>(this)->update();
}

void DataNode::doAddInput(DataNode* node)
{
    inputs.append(node);
}

void DataNode::doRemoveInput(DataNode* node)
{
    inputs.removeAll(node);
}

void DataNode::doAddOutput(DataNode* node)
{
    outputs.append(node);
}

void DataNode::doRemoveOutput(DataNode* node)
{
    outputs.removeAll(node);
}

} // namespace panda
