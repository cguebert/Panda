#ifndef DATA_H
#define DATA_H

#include <panda/BaseData.h>
#include <QVariant>

#include <QTextStream>
#include <QList>

namespace panda
{

QString dataTypeToName(int type);

BaseData* createDataFromType(int type, const QString& name, const QString& help, PandaObject* owner);
BaseData* createVectorDataFromType(int type, const QString& name, const QString& help, PandaObject* owner);
BaseData* createAnimationDataFromType(int type, const QString& name, const QString& help, PandaObject* owner);
BaseData* createDataFromFullType(int fullType, const QString& name, const QString& help, PandaObject* owner);

template<class T> QTextStream& readValue(QTextStream& stream, T& v);
template<class T> QTextStream& writeValue(QTextStream& stream, const T& v);

template <class T = void*>
class Data : public BaseData
{
public:
    typedef T value_type;

    class InitData : public BaseData::BaseInitData
    {
    public:
        InitData() : value(T()) {}
        T value;
    };

    explicit Data(const BaseData::BaseInitData& init)
        : BaseData(init)
        , value(T())
    {
		setCustomFlags();
    }

    explicit Data(const InitData& init)
        : BaseData(init)
    {
        value = init.value;
		setCustomFlags();
    }

    Data(const QString& name, const QString& help, PandaObject* owner)
		: BaseData(name, help, owner)
	{
		setCustomFlags();
	}

    virtual ~Data() {}

    virtual bool isSingleValue() const;
    virtual bool isVector() const;
    virtual bool isAnimation() const;

    virtual int getValueType() const;

    virtual int getSize() const;
    virtual void clear(int size = 0, bool init = false);

    virtual QVariant getBaseValue(int index) const;
    virtual void fromBaseValue(QVariant val, int index);

    virtual void fromString(const QString& text);

    virtual QString getFullTypeName() const;

    inline T* beginEdit()
    {
        this->updateIfDirty();
        ++counter;
        return &value;
    }

    inline void endEdit()
    {
        this->isValueSet = true;
        BaseData::setDirtyOutputs();
    }

    inline void setValue(const T& v)
    {
        *beginEdit() = v;
        endEdit();
    }

    inline const T& getValue() const
    {
        this->updateIfDirty();
        return value;
    }

    void copyValueFrom(const BaseData* parent);

protected:
    virtual QString doToString() const;

	void setCustomFlags();

private:
    T value;

    Data();
    Data(const Data&);
    Data& operator=(const Data&);
};

class GenericData : public Data<int>
{
public:
    explicit GenericData(const BaseData::BaseInitData& init)
		: Data<int>(init) {}

    virtual bool validParent(const BaseData* parent) const;
    virtual QString getFullTypeName() const;

	QList<int> allowedTypes;
};

class GenericNonVectorData : public GenericData
{
public:
    explicit GenericNonVectorData(const BaseData::BaseInitData& init)
        : GenericData(init) {}

    virtual bool validParent(const BaseData* parent) const;
    virtual QString getFullTypeName() const;
};

class GenericVectorData : public GenericData
{
public:
    explicit GenericVectorData(const BaseData::BaseInitData& init)
        : GenericData(init) {}

    virtual bool validParent(const BaseData* parent) const;
    virtual QString getFullTypeName() const;
};

class GenericAnimationData : public GenericData
{
public:
    explicit GenericAnimationData(const BaseData::BaseInitData& init)
        : GenericData(init) {}

    virtual bool validParent(const BaseData* parent) const;
    virtual QString getFullTypeName() const;
};

} // namespace panda

#endif // DATA_H
