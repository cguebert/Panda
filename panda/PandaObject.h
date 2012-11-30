#ifndef PANDAOBJECT_H
#define PANDAOBJECT_H

#include <panda/Data.h>
#include <panda/BaseClass.h>

#include <QObject>
#include <QSharedPointer>
#include <QPointF>

namespace panda
{

class PandaDocument;

class PandaObject : public QObject, public DataNode
{
    Q_OBJECT

public:
    explicit PandaObject(QObject *parent = 0);

	typedef TClass< PandaObject, void > MyClass;
	static const MyClass* getClass() { return MyClass::getInstance(); }
//	virtual const BaseClass* getBaseClass() const { return getClass(); }
	virtual const BaseClass* getBaseClass() const = 0;

    QString getName() const;
    quint32 getIndex() const;

    void addData(BaseData* data);
    void removeData(BaseData* data);

    void addOutput(BaseData* data);
    using DataNode::addOutput;

    BaseData* getData(const QString& name) const;
    QList<BaseData*> getDatas() const;
    QList<BaseData*> getInputDatas() const;
    QList<BaseData*> getOutputDatas() const;

    template<class TValue>
    BaseData::BaseInitData initData(Data<TValue>* data, QString name, QString help)
    {
        BaseData::BaseInitData init;
        init.data = data;
        init.name = name;
        init.help = help;
        init.owner = this;
        return init;
    }

    template<class TValue>
    typename Data<TValue>::InitData initData(Data<TValue>* data, const TValue& value, QString name, QString help)
    {
        typename Data<TValue>::InitData init;
        init.data = data;
        init.name = name;
        init.help = help;
        init.owner = this;
        init.value = value;
        return init;
    }

	virtual void postCreate() { }
    virtual void reset() { }
    virtual void update();
	virtual void updateIfDirty() const;
    virtual void setDirtyValue();

    virtual void save(QDataStream& out);
    virtual void save(QTextStream& out);

    virtual void load(QDataStream& in);
    virtual void load(QTextStream& in);

    virtual void dataSetParent(BaseData* data, BaseData* parent);

    void changeDataName(BaseData* data, const QString& newName);
	void emitModified();

protected:
    QString name;
    QList<BaseData*> datas;
    QMap<QString, BaseData*> datasMap;
    quint32 index;
	bool doEmitModified;
	mutable bool isUpdating;
	PandaDocument* parentDocument;

    void setInternalData(const QString& newName, const quint32 &newIndex);
    friend class ObjectFactory;
    friend class PandaDocument;

signals:
    void dirty(panda::PandaObject*);
    void modified(panda::PandaObject*);

public slots:

};

} // namespace Panda

#endif // PANDAOBJECT_H
