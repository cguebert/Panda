#ifndef DATA_H
#define DATA_H

#include <panda/BaseData.h>
#include <panda/DataAccessor.h>

namespace panda
{

template<class T> class DataAccessor;
template<class T> class Data;
template<class T> class RegisterData;

class AbstractDataCopier
{
public:
	virtual bool copyData(BaseData* dest, const BaseData* from) const = 0;
};

template <class T = void*>
class Data : public BaseData
{
public:
	PANDA_CLASS(PANDA_TEMPLATE(Data, T), BaseData)
	typedef T value_type;
	typedef value_type& reference;
	typedef const value_type& const_reference;
	typedef value_type* pointer;
	typedef Data<value_type> data_type;

	class InitData : public BaseData::BaseInitData
	{
	public:
		InitData() : value(value_type()) {}
		value_type value;
	};

	explicit Data(const BaseData::BaseInitData& init);
	explicit Data(const InitData& init);
	Data(const QString& name, const QString& help, PandaObject* owner);
	virtual ~Data();

	virtual void update();
	virtual void setParent(BaseData* parent);
	virtual const types::AbstractDataTrait* getDataTrait() const;
	virtual const void* getVoidValue() const;
	DataAccessor<data_type> getAccessor();
	inline void setValue(const_reference v);
	inline const_reference getValue() const;
	virtual void copyValueFrom(const BaseData* from);

protected:
	friend class DataAccessor<data_type>;
	friend class RegisterData<value_type>;

	inline pointer beginEdit();
	inline void endEdit();
	virtual void* beginVoidEdit();
	virtual void endVoidEdit();

private:
	value_type value;
	Data<value_type>* parentData;
	static types::AbstractDataTrait* dataTrait;
	static AbstractDataCopier* dataCopier;

	Data();
	Data(const Data&);
	Data& operator=(const Data&);
};

//***************************************************************//

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

} // namespace panda

#endif // DATA_H
