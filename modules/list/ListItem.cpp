#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <QMetaType>

namespace panda {

class ListItem : public PandaObject
{
public:
    ListItem(PandaDocument *doc)
        : PandaObject(doc)
        , indexData(initData(&indexData, "index", "0-based index of the items to extract from the lists"))
        , nbItems(initData(&nbItems, 1, "# items", "Minimum number of items in the lists"))
        , generic(initData(&generic, "input", "Connect here the lists to get the items from"))
    {
        addInput(&indexData);
        addInput(&generic);

        addOutput(&nbItems);

        generic.setDisplayed(false);
        generic.setPersistent(false);

        indexData.beginEdit()->append(0);
        indexData.endEdit();
    }

    ~ListItem()
    {
        foreach(BaseData* data, createdDatas.keys())
        {
            removeData(data);
            removeData(createdDatas[data]);

            delete createdDatas[data];
            delete data;
        }
    }

    BaseData* createDatas(int type)
    {
        int id = 1;
        foreach(BaseData* data, createdDatas.keys())
            if(data->getValueType() == type)
                id++;

        QString nameType, nameList, nameItem;
        nameType = BaseData::typeToName(type);
        nameList = QString(tr("list of %1 #%2")).arg(nameType).arg(id);
        nameItem = QString(tr("%1 item #%2")).arg(nameType).arg(id);

        BaseData *dataList, *dataItem;
        dataList = createVectorDataFromType(type, nameList, "", this);
        dataItem = createVectorDataFromType(type, nameItem, "", this);

        if(dataList && dataItem)
        {
            createdDatas.insert(dataList, dataItem);
            nonSortedDatas.append(dataList);

            addInput(dataList);
            addOutput(dataItem);

            removeData(&generic);	// generic must always be last
            addData(&generic);
        }

        return dataList;
    }

    void dataSetParent(BaseData* data, BaseData* parent)
    {
        if(data == &generic)
        {
            int type = parent->getValueType();
            BaseData *dataList = createDatas(type);

            if(dataList)
                dataList->setParent(parent);

            emit modified(this);
        }
        else if(parent || !createdDatas.contains(data))
            data->setParent(parent);
        else // (NULL), we remove the data
        {
            int type = data->getValueType();
            BaseData* dataItem = createdDatas[data];
            data->setParent(NULL);

            createdDatas.remove(data);

            removeData(data);
            removeData(dataItem);

            nonSortedDatas.removeAll(data);

            delete data;
            delete dataItem;

            int id=0;
            foreach(BaseData* data, createdDatas.keys())
            {
                if(data->getValueType() == type)
                {
                    ++id;
                    QString nameType, nameList, nameItem;
                    nameType = BaseData::typeToName(type);
                    nameList = QString(tr("list of %1 #%2")).arg(nameType).arg(id);
                    nameItem = QString(tr("%1 item #%2")).arg(nameType).arg(id);

                    data->setName(nameList);
                    createdDatas[data]->setName(nameItem);
                }
            }

            emit modified(this);
        }
    }

    void update()
    {
        const QVector<int>& id = indexData.getValue();
        int idSize = id.size();
        int nb = -1;
        foreach(BaseData* data, createdDatas.keys())
        {
            data->updateIfDirty();
            createdDatas[data]->clear(idSize);

            int size = data->getSize();
            if(nb > 0)	nb = qMin(nb, size);
            else		nb = size;
        }
        nb = qMax(0, nb);
        if(nb != nbItems.getValue())
            nbItems.setValue(nb);

        if(idSize)
        {
            for(int i=0; i<idSize; ++i)
            {
                foreach(BaseData* data, createdDatas.keys())
                    createdDatas[data]->fromBaseValue(data->getBaseValue(id[i]), i);
            }
        }

        this->cleanDirty();
    }

    void save(QDataStream& out)
    {
        out << (quint32)nonSortedDatas.size();
        foreach(BaseData* data, nonSortedDatas)
            out << (quint32)data->getValueType();

        PandaObject::save(out);
    }

    void save(QTextStream& out)
    {
        out << (quint32)nonSortedDatas.size();
        foreach(BaseData* data, nonSortedDatas)
            out << (quint32)data->getValueType();

        PandaObject::save(out);
    }

    void load(QDataStream& in)
    {
        quint32 nb;
        in >> nb;

        for(quint32 i=0; i<nb; ++i)
        {
            quint32 type;
            in >> type;
            createDatas(type);
        }

        PandaObject::load(in);
    }

    void load(QTextStream& in)
    {
        quint32 nb;
        in >> nb;

        for(quint32 i=0; i<nb; ++i)
        {
            quint32 type;
            in >> type;
            createDatas(type);
        }

        in.skipWhiteSpace();
        PandaObject::load(in);
    }

protected:
    Data< QVector<int> > indexData;
    Data<int> nbItems;
    GenericVectorData generic;
    QList<BaseData*> nonSortedDatas;
    QMap<BaseData*, BaseData*> createdDatas;
};

int ListItemClass = RegisterObject("List/List item").setClass<ListItem>().setDescription("Extract an item from a list");

} // namespace Panda
