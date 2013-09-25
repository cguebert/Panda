#ifndef DATA_H
#define DATA_H

#include <panda/BaseData.h>
#include <panda/DataTraits.h>
#include <helper/DataAccessor.h>

#include <QVariant>
#include <QTextStream>

namespace panda
{

template<class T> QTextStream& readValue(QTextStream& stream, T& v);
template<class T> QTextStream& writeValue(QTextStream& stream, const T& v);

template<class T> class DataAccessor;

template <class T = void*>
class Data : public BaseData
{
public:
	PANDA_CLASS(PANDA_TEMPLATE(Data, T), BaseData)
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
		displayed = data_trait<T>::isDisplayed();
		persistent = data_trait<T>::isPersistent();
    }

    explicit Data(const InitData& init)
        : BaseData(init)
    {
        value = init.value;
		displayed = data_trait<T>::isDisplayed();
		persistent = data_trait<T>::isPersistent();
    }

    Data(const QString& name, const QString& help, PandaObject* owner)
		: BaseData(name, help, owner)
	{
		displayed = data_trait<T>::isDisplayed();
		persistent = data_trait<T>::isPersistent();
	}

	virtual ~Data() {}

	virtual bool isSingleValue() const
	{ return data_trait<T>::is_single; }

	virtual bool isVector() const
	{ return data_trait<T>::is_vector; }

	virtual bool isAnimation() const
	{ return data_trait<T>::is_animation; }

	virtual int getValueType() const
	{ return data_trait<T>::valueType(); }

	virtual QString getValueTypeName() const
	{ return data_trait<T>::valueTypeName(); }

	virtual int getSize() const
	{ return data_trait<T>::size(*this); }

	virtual void clear(int size = 0, bool init = false)
	{ data_trait<T>::clear(*this, size, init); }

	virtual QVariant getBaseValue(int index) const
	{ return data_trait<T>::getBaseValue(*this, index); }

	virtual void fromBaseValue(QVariant val, int index)
	{ data_trait<T>::fromBaseValue(*this, val, index); }

	virtual void fromString(const QString& text)
	{
		beginEdit();
		value = valueFromString<T>(text);
		endEdit();
	}

	helper::DataAccessor< Data<value_type> > getAccessor()
	{ return helper::DataAccessor< Data<T> >(*this); }

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

	virtual void copyValueFrom(const BaseData* parent)
	{
		data_trait<T>::copyValue(this, parent);
		this->isValueSet = true;
	}

	virtual void save(QDomDocument& doc, QDomElement& elem)
	{ data_trait<T>::writeValue(doc, elem, value); }

	virtual void load(QDomElement& elem)
	{ data_trait<T>::readValue(elem, value); }

protected:
	virtual QString doToString() const
	{ return valueToString(value); }

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
