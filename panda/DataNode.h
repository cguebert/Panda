#ifndef DATANODE_H
#define DATANODE_H

#include <QList>

namespace panda
{

class BaseData;

class DataNode
{
public:
    typedef QList<DataNode*> NodesList;

    DataNode();
    virtual ~DataNode();

    void addInput(DataNode* node);
    void removeInput(DataNode* node);
    void addOutput(DataNode* node);
    void removeOutput(DataNode* node);

    const NodesList& getInputs();
    const NodesList& getOutputs();

    virtual void update() = 0;

    bool isDirty() const;
    virtual void setDirtyValue();
    virtual void setDirtyOutputs();
    void cleanDirty();
    void updateIfDirty() const;

protected:
    virtual void doAddInput(DataNode* node);
    virtual void doRemoveInput(DataNode* node);
    virtual void doAddOutput(DataNode* node);
    virtual void doRemoveOutput(DataNode* node);

	bool dirtyValue;
    NodesList inputs, outputs;
};

} // namespace panda

#endif // DATANODE_H
