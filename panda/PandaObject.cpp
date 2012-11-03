#include <panda/PandaObject.h>
#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>

namespace panda {

PandaObject::PandaObject(QObject *parent)
    : QObject(parent)
	, doEmitModified(true)
{
}

QString PandaObject::getName() const
{
    return name;
}

quint32 PandaObject::getIndex() const
{
    return index;
}

void PandaObject::addData(BaseData* data)
{
    if(datasMap.contains(data->getName()))
    {
        std::cerr << "Fatal error : another data already have the name " << data->getName().toStdString() << std::endl;
        return;
    }
    if(!datas.contains(data))
    {
        datas.append(data);
        datasMap[data->getName()] = data;
		emitModified();
    }
}

void PandaObject::removeData(BaseData* data)
{
    datasMap.remove(data->getName());
    if(datas.removeAll(data))
		emitModified();
}

void PandaObject::addOutput(BaseData* data)
{
    data->setReadOnly(true);
    DataNode::addOutput(data);
}

void PandaObject::update()
{
    this->cleanDirty();
}

void PandaObject::setDirtyValue()
{
    if(!dirtyValue)
    {
        DataNode::setDirtyValue();
        emit dirty(this);
    }
}

BaseData* PandaObject::getData(const QString& name) const
{
    if(datasMap.contains(name))
        return datasMap[name];
    else return NULL;
}

QList<BaseData*> PandaObject::getDatas() const
{
    return datas;
}

QList<BaseData*> PandaObject::getInputDatas() const
{
    QList<BaseData*> temp;
    foreach(BaseData* data, datas)
    {
        if(data->isInput())
            temp.append(data);
    }

    return temp;
}

QList<BaseData*> PandaObject::getOutputDatas() const
{
    QList<BaseData*> temp;
    foreach(BaseData* data, datas)
    {
        if(data->isOutput())
            temp.append(data);
    }

    return temp;
}

void PandaObject::setInternalData(const QString& newName, const quint32& newIndex)
{
    name = newName;
    index = newIndex;
}

void PandaObject::save(QDataStream& out)
{
    int nb = 0;
    foreach(BaseData* data, datas)
        if(data->isSet() && data->isPersistent() && !data->isReadOnly() && !data->getParent())
            ++nb;

    out << nb;
    foreach(BaseData* data, datas)
    {
        if(data->isSet() && data->isPersistent() && !data->isReadOnly() && !data->getParent())
        {
            out << data->getName();
            out << data->toString();
        }
    }
}

void PandaObject::save(QTextStream& out)
{
    int nb = 0;
    foreach(BaseData* data, datas)
        if(data->isSet() && data->isPersistent() && !data->isReadOnly() && !data->getParent())
            ++nb;

    out << nb << endl;
    foreach(BaseData* data, datas)
    {
        if(data->isSet() && data->isPersistent() && !data->isReadOnly() && !data->getParent())
        {
            out << data->getName() << endl;
            out << data->toString() << endl;
        }
    }
}

void PandaObject::load(QDataStream& in)
{
    int nb;
    in >> nb;

    for(int i=0; i<nb; ++i)
    {
        QString name, value;
        in >> name >> value;
        BaseData* data = getData(name);
        if(data)
            data->fromString(value);
    }
}

void PandaObject::load(QTextStream& in)
{
    int nb;
    in >> nb;

    for(int i=0; i<nb; ++i)
    {
        QString name, value;
        in.skipWhiteSpace();
        name = in.readLine();
        in.skipWhiteSpace();
        value = in.readLine();
        BaseData* data = getData(name);
        if(data)
            data->fromString(value);
    }
}

void PandaObject::dataSetParent(BaseData* data, BaseData* parent)
{
    data->setParent(parent);
}

void PandaObject::changeDataName(BaseData* data, const QString& newName)
{
    datasMap.remove(data->getName());
    datasMap.insert(newName, data);
}

void PandaObject::emitModified()
{
	if(doEmitModified)
		emit modified(this);
}

} // namespace Panda


