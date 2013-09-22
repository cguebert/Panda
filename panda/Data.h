#ifndef DATA_H
#define DATA_H

#include <panda/BaseData.h>
#include <helper/DataAccessor.h>

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

template<class T> class DataAccessor;

template <class T = void*>
class Data : public BaseData
{
public:
	typedef T value_type;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T* pointer;

    class InitData : public BaseData::BaseInitData
    {
    public:
		InitData() : value(value_type()) {}
		value_type value;
    };

    explicit Data(const BaseData::BaseInitData& init)
        : BaseData(init)
		, value(value_type())
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

	helper::DataAccessor< Data<value_type> > getAccessor();

	inline void setValue(const_reference v)
    {
        *beginEdit() = v;
        endEdit();
    }

	inline const_reference getValue() const
    {
        this->updateIfDirty();
        return value;
    }

	virtual void copyValueFrom(const BaseData* parent);

	virtual void save(QDomDocument& doc, QDomElement& elem);

	virtual void load(QDomElement& elem);

protected:
    virtual QString doToString() const;

	void setCustomFlags();

	friend class helper::DataAccessor< Data<T> >;

	inline pointer beginEdit()
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

private:
    T value;

    Data();
    Data(const Data&);
    Data& operator=(const Data&);
};

//***************************************************************//

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

//***************************************************************//

namespace helper
{

template<class T>
class DataAccessor<Data<T>> : public DataAccessor<T>
{
public:
	typedef DataAccessor<T> Inherit;
	typedef panda::Data<T> data_type;
	typedef T container_type;

protected:
	data_type& data;

public:
	DataAccessor(data_type& d) : Inherit(*d.beginEdit()) , data(d) {}
	~DataAccessor() { data.endEdit(); }

	template<class U> void operator=(const U& v) { Inherit::operator=(v); }
};

} // namespace helper

} // namespace panda

#endif // DATA_H
