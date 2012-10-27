#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Animation.h>

#include <QMetaType>

namespace panda {

class AnimValue : public PandaObject
{
public:
    AnimValue(PandaDocument *doc)
        : PandaObject(doc)
        , progress(initData(&progress, "progress", "Position inside the animation"))
        , generic(initData(&generic, "input", "Connect here the animations to get the values from"))
        , maxLength(initData(&maxLength, 0.0, "max length", "Length of the longuest animation"))
    {
        addInput(&progress);
        addInput(&generic);

        addOutput(&maxLength);

        generic.setDisplayed(false);
        generic.setPersistent(false);

        progress.beginEdit()->append(0.0);
        progress.endEdit();
    }

    ~AnimValue()
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

        QString nameType, nameAnim, nameValue;
        nameType = BaseData::typeToName(type);
        nameAnim = QString(tr("animation of %1 #%2")).arg(nameType).arg(id);
        nameValue = QString(tr("%1 value #%2")).arg(nameType).arg(id);

        BaseData *dataAnim, *dataValue;
        dataAnim = createAnimationDataFromType(type, nameAnim, "", this);
        dataValue = createVectorDataFromType(type, nameValue, "", this);

        if(dataAnim && dataValue)
        {
            createdDatas.insert(dataAnim, dataValue);
            nonSortedDatas.append(dataAnim);

            addInput(dataAnim);
            addOutput(dataValue);

            removeData(&generic);	// generic must always be last
            addData(&generic);
        }

        return dataAnim;
    }

    void dataSetParent(BaseData* data, BaseData* parent)
    {
        if(data == &generic)
        {
            int type = parent->getValueType();
            BaseData *dataAnim = createDatas(type);

            if(dataAnim)
                dataAnim->setParent(parent);

            emit modified(this);
        }
        else if(parent || !createdDatas.contains(data))
            data->setParent(parent);
        else // (NULL), we remove the data
        {
            int type = data->getValueType();
            BaseData* dataValue = createdDatas[data];
            data->setParent(NULL);

            createdDatas.remove(data);

            removeData(data);
            removeData(dataValue);

            nonSortedDatas.removeAll(data);

            delete data;
            delete dataValue;

            int id=0;
            foreach(BaseData* data, createdDatas.keys())
            {
                if(data->getValueType() == type)
                {
                    ++id;
                    QString nameType, nameAnim, nameValue;
                    nameType = BaseData::typeToName(type);
                    nameAnim = QString(tr("animation of %1 #%2")).arg(nameType).arg(id);
                    nameValue = QString(tr("%1 value #%2")).arg(nameType).arg(id);

                    data->setName(nameAnim);
                    createdDatas[data]->setName(nameValue);
                }
            }

            emit modified(this);
        }
    }

    double getLength(BaseData* data)
    {
        switch(data->getValueType())
        {
        case QMetaType::Double:
            {
                Data< Animation<double> >* animDouble = dynamic_cast<Data< Animation<double> >*>(data);
                if(animDouble)
                {
                    const Animation<double>& valAnim = animDouble->getValue();
                    if(valAnim.size())
                        return valAnim.getKeys().back();
                    else
                        return 0.0;
                }
                break;
            }
        case QMetaType::QColor:
            {
                Data< Animation<QColor> >* animColor = dynamic_cast<Data< Animation<QColor> >*>(data);
                if(animColor)
                {
                    const Animation<QColor>& valAnim = animColor->getValue();
                    if(valAnim.size())
                        return valAnim.getKeys().back();
                    else
                        return 0.0;
                }
                break;
            }
        case QMetaType::QPointF:
            {
                Data< Animation<QPointF> >* animPoint = dynamic_cast<Data< Animation<QPointF> >*>(data);
                if(animPoint)
                {
                    const Animation<QPointF>& valAnim = animPoint->getValue();
                    if(valAnim.size())
                        return animPoint->getValue().getKeys().back();
                    else
                        return 0.0;
                }
                break;
            }
        }

        return 0;
    }

    void copyValue(BaseData* dataAnim, BaseData* dataValue, const QVector<double>& key)
    {
        int keySize = key.size();
        switch(dataAnim->getValueType())
        {
        case QMetaType::Double:
            {
                Data< Animation<double> >* dataAnimDouble = dynamic_cast<Data< Animation<double> >*>(dataAnim);
                Data< QVector<double> >* dataValDouble = dynamic_cast<Data< QVector<double> >*>(dataValue);
                if(dataAnimDouble && dataValDouble)
                {
                    const Animation<double>& animDouble = dataAnimDouble->getValue();
                    QVector<double>& valDouble = *dataValDouble->beginEdit();
//					valDouble.resize(keySize);
                    for(int i=0; i<keySize; ++i)
                        valDouble[i] = animDouble.get(key[i]);
                    dataValDouble->endEdit();
                }
                break;
            }
        case QMetaType::QColor:
            {
                Data< Animation<QColor> >* dataAnimColor = dynamic_cast<Data< Animation<QColor> >*>(dataAnim);
                Data< QVector<QColor> >* dataValColor = dynamic_cast<Data< QVector<QColor> >*>(dataValue);
                if(dataAnimColor && dataValColor)
                {
                    const Animation<QColor>& animColor = dataAnimColor->getValue();
                    QVector<QColor>& valColor = *dataValColor->beginEdit();
//					valColor.resize(keySize);
                    for(int i=0; i<keySize; ++i)
                        valColor[i] = animColor.get(key[i]);
                    dataValColor->endEdit();
                }
                break;
            }
        case QMetaType::QPointF:
            {
                Data< Animation<QPointF> >* dataAnimPoint = dynamic_cast<Data< Animation<QPointF> >*>(dataAnim);
                Data< QVector<QPointF> >* dataValPoint = dynamic_cast<Data< QVector<QPointF> >*>(dataValue);
                if(dataAnimPoint && dataValPoint)
                {
                    const Animation<QPointF>& animPoint = dataAnimPoint->getValue();
                    QVector<QPointF>& valPoint = *dataValPoint->beginEdit();
//					valPoint.resize(keySize);
                    for(int i=0; i<keySize; ++i)
                        valPoint[i] = animPoint.get(key[i]);
                    dataValPoint->endEdit();
                }
                break;
            }
        }
    }

    void update()
    {
        const QVector<double>& key = progress.getValue();
        int keySize = key.size();
        double len = 0;
        foreach(BaseData* data, createdDatas.keys())
        {
            data->updateIfDirty();
            createdDatas[data]->clear(keySize);

            double l = getLength(data);
            len = qMax(len, l);
        }

        if(len != maxLength.getValue())
            maxLength.setValue(len);

        if(keySize)
        {
            foreach(BaseData* data, createdDatas.keys())
                copyValue(data, createdDatas[data], key);
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
    Data< QVector<double> > progress;
    GenericAnimationData generic;
    Data<double> maxLength;
    QList<BaseData*> nonSortedDatas;
    QMap<BaseData*, BaseData*> createdDatas;
};

int AnimValueClass = RegisterObject("Animation/Animation value").setClass<AnimValue>().setName("Anim value").setDescription("Extract a value from an animation");

} // namespace Panda
