#ifndef TYPES_DATATRAITS_H
#define TYPES_DATATRAITS_H

#include <panda/types/DataTypeId.h>

#include <QVector>
#include <QDomDocument>

namespace panda
{

namespace types
{

class AbstractDataTrait
{
public:
	virtual const AbstractDataTrait* baseTypeTrait() const = 0;
	virtual const AbstractDataTrait* valueTypeTrait() const = 0;

	virtual bool isSingleValue() const = 0;
	virtual bool isVector() const = 0;
	virtual bool isAnimation() const = 0;
	virtual bool isDisplayed() const = 0;
	virtual bool isPersistent() const = 0;

	virtual QString valueTypeName() const = 0;			/// Readable form of the value type ("integer")
	virtual QString valueTypeNamePlural() const = 0;	/// Plural form ("integers")
	virtual QString typeName() const = 0;				/// When we save the type ("integer_vector")
	virtual QString typeDescription() const = 0;		/// Full display string ("vector of integers")
	virtual int valueTypeId() const = 0;
	virtual int fullTypeId() const = 0;

	virtual int size(const void* value) const = 0;
	virtual void clear(void* value, int size, bool init) const = 0;
	virtual const void* getVoidValue(const void* value, int index) const = 0;
	virtual void* getVoidValue(void* value, int index) const = 0;

	virtual void writeValue(QDomDocument& doc, QDomElement& elem, const void* value) const = 0;
	virtual void readValue(QDomElement& elem, void* value) const = 0;
};

//****************************************************************************//

/*
 * Class used to describe a type
 * 3 functions have to be written for each type:
 *   valueTypeName, writeValue & readValue
 */
template<class T>
class DataTrait
{
public:
	typedef T base_type;
	typedef T value_type;

	enum { is_single = 1 };
	enum { is_vector = 0 };
	enum { is_animation = 0 };
	static bool isDisplayed() { return true; }
	static bool isPersistent() { return true; }

	static QString valueTypeName(); // Override for each type
	static QString valueTypeNamePlural() { return valueTypeName() + "s"; }
	static QString typeName() { return valueTypeName(); }
	static QString typeDescription() { return valueTypeName() + " value"; }
	static int valueTypeId() { return DataTypeId::getIdOf<value_type>(); }
	static int fullTypeId() { return DataTypeId::getFullTypeOfSingleValue(valueTypeId()); }
	static int size(const value_type& /*v*/) { return 1; }
	static void clear(value_type& v, int /*size*/, bool init) { if(init) v = T(); }
	static const void* getVoidValue(const value_type& v, int /*index*/) { return &v; }
	static void* getVoidValue(value_type& v, int /*index*/) { return &v; }
	static void writeValue(QDomDocument&, QDomElement&, const value_type&); // Override for each type
	static void readValue(QDomElement&, value_type&); // Override for each type
};

//****************************************************************************//

template<class T>
class VirtualDataTrait : public AbstractDataTrait
{
public:
	typedef T value_type;
	typedef DataTrait<value_type> value_trait;

	static VirtualDataTrait* get() { static VirtualDataTrait<value_type> trait; return &trait; }

	virtual const AbstractDataTrait* baseTypeTrait() const
	{ return VirtualDataTrait<value_trait::base_type>::get(); }
	virtual const AbstractDataTrait* valueTypeTrait() const
	{ return VirtualDataTrait<value_trait::value_type>::get(); }

	virtual bool isSingleValue() const	{ return value_trait::is_single; }
	virtual bool isVector() const		{ return value_trait::is_vector; }
	virtual bool isAnimation() const	{ return value_trait::is_animation; }
	virtual bool isDisplayed() const	{ return value_trait::isDisplayed(); }
	virtual bool isPersistent() const	{ return value_trait::isPersistent(); }

	virtual QString valueTypeName() const { return value_trait::valueTypeName(); }
	virtual QString valueTypeNamePlural() const { return value_trait::valueTypeNamePlural(); }
	virtual QString typeName() const { return value_trait::typeName(); }
	virtual QString typeDescription() const { return value_trait::typeDescription(); }
	virtual int valueTypeId() const { return value_trait::valueTypeId(); }
	virtual int fullTypeId() const { return value_trait::fullTypeId(); }

	virtual int size(const void* value) const
	{ return value_trait::size(*static_cast<const value_type*>(value)); }
	virtual void clear(void* value, int size, bool init) const
	{ return value_trait::clear(*static_cast<value_type*>(value), size, init); }
	virtual const void* getVoidValue(const void* value, int index) const
	{ return value_trait::getVoidValue(*static_cast<const value_type*>(value), index); }
	virtual void* getVoidValue(void* value, int index) const
	{ return value_trait::getVoidValue(*static_cast<value_type*>(value), index); }

	virtual void writeValue(QDomDocument& doc, QDomElement& elem, const void* value) const
	{ return value_trait::writeValue(doc, elem, *static_cast<const value_type*>(value)); }
	virtual void readValue(QDomElement& elem, void* value) const
	{ return value_trait::readValue(elem, *static_cast<value_type*>(value)); }
};

//****************************************************************************//

template<class T>
class DataTrait< QVector<T> >
{
public:
	typedef QVector<T> vector_type;
	typedef T base_type;
	typedef T value_type;
	typedef DataTrait<base_type> base_trait;

	enum { is_single = 0 };
	enum { is_vector = 1 };
	enum { is_animation = 0 };
	static bool isDisplayed() { return base_trait::isDisplayed(); }
	static bool isPersistent() { return base_trait::isPersistent(); }

	static QString valueTypeName() { return base_trait::valueTypeName(); }
	static QString valueTypeNamePlural() { return base_trait::valueTypeNamePlural(); }
	static QString typeName() { return valueTypeName() + "_vector"; }
	static QString typeDescription() { return "vector of " + valueTypeNamePlural(); }
	static int valueTypeId() { return DataTypeId::getIdOf<value_type>(); }
	static int fullTypeId() { return DataTypeId::getFullTypeOfVector(valueTypeId()); }
	static int size(const vector_type& v) { return v.size(); }
	static void clear(vector_type& v, int size, bool init)
	{
		if(init)
			v.clear();
		v.resize(size);
	}
	static const void* getVoidValue(const vector_type& vec, int index)
	{
		if(index < 0 || index >= vec.size())
			return nullptr;
		return &vec[index];
	}
	static void* getVoidValue(vector_type& vec, int index)
	{
		if(index < 0 || index >= vec.size())
			return nullptr;
		return &vec[index];
	}
	static void writeValue(QDomDocument& doc, QDomElement& elem, const vector_type& vec)
	{
		for(auto& v : vec)
		{
			QDomElement node = doc.createElement("Value");
			base_trait::writeValue(doc, node, v);
			elem.appendChild(node);
		}
	}
	static void readValue(QDomElement& elem, vector_type& vec)
	{
		vec.clear();
		T t = T();
		QDomElement e = elem.firstChildElement("Value");
		while(!e.isNull())
		{
			base_trait::readValue(e, t);
			vec.push_back(t);
			e = e.nextSiblingElement("Value");
		}
	}
};

//****************************************************************************//

class DataTraitsList
{
public:
	static AbstractDataTrait* getTrait(int fullTypeId);
	static AbstractDataTrait* getTrait(const std::type_info& type);
	template <class T>
	static AbstractDataTrait* getTraitOf() { return getTrait(typeid(T)); }

private:
	DataTraitsList();

	template<class T> friend class RegisterData;

	static void registerTrait(AbstractDataTrait* trait);
};

} // namespace types

} // namespace panda

#endif // TYPES_DATATRAITS_H
