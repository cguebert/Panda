#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <QMetaType>

namespace panda {

class FillList : public PandaObject
{
public:
    FillList(PandaDocument *doc)
        : PandaObject(doc)
        , size(initData(&size, 10, "size", "Size of the list to be created"))
        , generic(initData(&generic, "input", "Connect here the lists to get the items from"))
    {
        addInput(&size);
        addInput(&generic);

        generic.setDisplayed(false);
        generic.setPersistent(false);
    }

    ~FillList()
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
        nameList = QString(tr("%1 item #%2")).arg(nameType).arg(id);
        nameItem = QString(tr("list of %1 #%2")).arg(nameType).arg(id);

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
		static bool protection = false;
		if(protection)
			return;

        if(data == &generic)
        {
            int type = parent->getValueType();
            BaseData *dataList = createDatas(type);

            if(dataList)
                dataList->setParent(parent);

            emit modified(this);
        }
        else if(parent || !createdDatas.contains(data))
		{
			protection = true;
			data->setParent(parent);
			protection = false;
		}
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
                    nameList = QString(tr("%1 value #%2")).arg(nameType).arg(id);
                    nameItem = QString(tr("list of %1 #%2")).arg(nameType).arg(id);

                    data->setName(nameList);
                    createdDatas[data]->setName(nameItem);
                }
            }

            emit modified(this);
        }
    }

    void update()
    {
        int outputSize = size.getValue();
        foreach(BaseData* data, createdDatas.keys())
        {
            data->updateIfDirty();
            createdDatas[data]->clear(outputSize);
        }

        if(outputSize)
        {
            foreach(BaseData* data, createdDatas.keys())
            {
                BaseData* outputData = createdDatas[data];
                int inputSize = data->getSize();
                if(inputSize)
                {
                    for(int i=0; i<outputSize; ++i)
                        outputData->fromBaseValue(data->getBaseValue(i % inputSize), i);
                }
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
		out << (quint32)nonSortedDatas.size() << " ";
        foreach(BaseData* data, nonSortedDatas)
			out << (quint32)data->getValueType() << " ";
		out << endl;

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
    Data<int> size;
    GenericData generic;
    QList<BaseData*> nonSortedDatas;
    QMap<BaseData*, BaseData*> createdDatas;
};

int FillListClass = RegisterObject("List/Fill list").setClass<FillList>().setDescription("Fill a list with a value");

} // namespace Panda
