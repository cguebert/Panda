#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Animation.h>

#include <QMetaType>

namespace panda {

class List2Anim : public PandaObject
{
public:
    List2Anim(PandaDocument *doc)
        : PandaObject(doc)
        , interval(initData(&interval, 1.0, "interval", "Interval between 2 animation keys"))
        , maxLength(initData(&maxLength, 0.0, "max length", "Length of the longuest animation"))
        , generic(initData(&generic, "input", "Connect here the lists to get the values from"))
    {
        addInput(&interval);
        addInput(&generic);

        addOutput(&maxLength);

        generic.setDisplayed(false);
        generic.setPersistent(false);

		generic.allowedTypes.append(BaseData::getValueTypeOf<double>());
		generic.allowedTypes.append(BaseData::getValueTypeOf<QPointF>());
		generic.allowedTypes.append(BaseData::getValueTypeOf<QColor>());
    }

    BaseData* createDatas(int type)
    {
        int index = 1;
        foreach(BaseData* data, createdDatas.keys())
            if(data->getValueType() == type)
                index++;

        QString nameType, nameList, nameAnim;
        nameType = BaseData::typeToName(type);
        nameList = QString(tr("list of %1 #%2")).arg(nameType).arg(index);
        nameAnim = QString(tr("animation of %1 #%2")).arg(nameType).arg(index);

        BaseData *dataList, *dataAnim;
        dataList = createVectorDataFromType(type, nameList, "", this);
        dataAnim = createAnimationDataFromType(type, nameAnim, "", this);

        if(dataList && dataAnim)
        {
            createdDatas.insert(dataList, dataAnim);
            nonSortedDatas.append(dataList);

            addInput(dataList);
            addOutput(dataAnim);

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
            BaseData* dataAnim = createdDatas[data];
            data->setParent(NULL);

            createdDatas.remove(data);

            removeData(data);
            removeData(dataAnim);

            nonSortedDatas.removeAll(data);

            delete data;
            delete dataAnim;

            int index=0;
            foreach(BaseData* data, createdDatas.keys())
            {
                if(data->getValueType() == type)
                {
                    ++index;
                    QString nameType, nameList, nameAnim;
                    nameType = BaseData::typeToName(type);
                    nameList = QString(tr("list of %1 #%2")).arg(nameType).arg(index);
                    nameAnim = QString(tr("animation of %1 #%2")).arg(nameType).arg(index);

                    data->setName(nameList);
                    createdDatas[data]->setName(nameAnim);
                }
            }

            emit modified(this);
        }
    }

    void createAnimation(BaseData* dataList, BaseData* dataAnim, double inter)
    {
        switch(dataList->getValueType())
        {
        case QMetaType::Double:
            {
                Data< QVector<double> >* listDoubleData = dynamic_cast<Data< QVector<double> >*>(dataList);
                Data< Animation<double> >* animDoubleData = dynamic_cast<Data< Animation<double> >*>(dataAnim);
                if(animDoubleData && listDoubleData)
                {
                    const QVector<double> listDouble = listDoubleData->getValue();
                    Animation<double>* animDouble = animDoubleData->beginEdit();
                    int size = listDouble.size();
                    animDouble->clear();
                    for(int i=0; i<size; ++i)
                        animDouble->add(i*inter, listDouble[i]);
                    animDoubleData->endEdit();
                }
                break;
            }
        case QMetaType::QColor:
            {
                Data< QVector<QColor> >* listColorData = dynamic_cast<Data< QVector<QColor> >*>(dataList);
                Data< Animation<QColor> >* animColorData = dynamic_cast<Data< Animation<QColor> >*>(dataAnim);
                if(animColorData && listColorData)
                {
                    const QVector<QColor> listColor = listColorData->getValue();
                    Animation<QColor>* animColor = animColorData->beginEdit();
                    int size = listColor.size();
                    animColor->clear();
                    for(int i=0; i<size; ++i)
                        animColor->add(i*inter, listColor[i]);
                    animColorData->endEdit();
                }
                break;
            }
        case QMetaType::QPointF:
            {
                Data< QVector<QPointF> >* listPointData = dynamic_cast<Data< QVector<QPointF> >*>(dataList);
                Data< Animation<QPointF> >* animPointData = dynamic_cast<Data< Animation<QPointF> >*>(dataAnim);
                if(animPointData && listPointData)
                {
                    const QVector<QPointF> listPoint = listPointData->getValue();
                    Animation<QPointF>* animPoint = animPointData->beginEdit();
                    int size = listPoint.size();
                    animPoint->clear();
                    for(int i=0; i<size; ++i)
                        animPoint->add(i*inter, listPoint[i]);
                    animPointData->endEdit();
                }
                break;
            }
        }
    }

    void update()
    {
        double inter = interval.getValue();
        int size = 0;
        foreach(BaseData* data, createdDatas.keys())
        {
            data->updateIfDirty();

            int s = data->getSize();
            size = qMax(size, s);
        }

        double len = size * inter;
        if(len != maxLength.getValue())
            maxLength.setValue(len);

        foreach(BaseData* data, createdDatas.keys())
            createAnimation(data, createdDatas[data], inter);

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
    Data<double> interval, maxLength;
    GenericVectorData generic;
    QList<BaseData*> nonSortedDatas;
    QMap<BaseData*, BaseData*> createdDatas;
};

int List2AnimClass = RegisterObject("Animation/List to Animation").setClass<List2Anim>().setName("List 2 Anim").setDescription("Create animations from lists");

} // namespace Panda
