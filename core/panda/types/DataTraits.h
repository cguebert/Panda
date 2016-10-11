#ifndef TYPES_DATATRAITS_H
#define TYPES_DATATRAITS_H

#include <panda/core.h>
#include <panda/types/DataTypeId.h>
#include <panda/XmlDocument.h>

#include <vector>

namespace panda
{

namespace types
{

class PANDA_CORE_API AbstractDataTrait
{
public:
	virtual const AbstractDataTrait* baseTypeTrait() const = 0;
	virtual const AbstractDataTrait* valueTypeTrait() const = 0;

	virtual bool isSingleValue() const = 0;					/// Contains only one value of the base type
	virtual bool isVector() const = 0;						/// Is a std::vector of the base type
	virtual bool isAnimation() const = 0;					/// Is an panda::types::Animation of the base type
	virtual bool isDisplayed() const = 0;					/// Can it be edited in the GUI (for example, Images are not displayed)
	virtual bool isPersistent() const = 0;					/// Do we save the value when saving the document and copy-pasting (Image are also not persistent because of their size)

	virtual std::string valueTypeName() const = 0;			/// Readable form of the value type ("integer")
	virtual std::string valueTypeNamePlural() const = 0;	/// Plural form ("integers")
	virtual std::string typeName() const = 0;				/// When we save the type ("integer_vector")
	virtual std::string typeDescription() const = 0;		/// Full display string ("vector of integers")
	virtual const std::type_info& typeInfo() const = 0;
	virtual int valueTypeId() const = 0;					/// The index of the value type in the DataTraitsList
	virtual int fullTypeId() const = 0;						/// The index of the type in the DataTraitsList
	virtual unsigned int typeColor() const = 0;				/// Unique color used to draw the data in the GUI

	virtual int size(const void* value) const = 0;								/// Returns the size of the vector or animation (if single value, returns 1)
	virtual void clear(void* value, int size, bool init) const = 0;				/// Clear a vector or animation (does nothing if single value, even for Path)
	virtual const void* getVoidValue(const void* value, int index) const = 0;	/// Returns a pointer to the value at this index
	virtual void* getVoidValue(void* value, int index) const = 0;				/// Returns a pointer to the value at this index

	virtual void writeValue(XmlElement& elem, const void* value) const = 0; /// Save the value to XML
	virtual void readValue(const XmlElement& elem, void* value) const = 0;		/// Load the value from XML
};

//****************************************************************************//

/*
 * Class used to describe a type
 * 4 functions have to be written for each type:
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

	// Override for each type
	static std::string valueTypeName(); 
	static unsigned int typeColor();
	static void writeValue(XmlElement&, const value_type&);
	static void readValue(const XmlElement&, value_type&);

	// Default implementation
	static std::string valueTypeNamePlural() { return valueTypeName() + "s"; }
	static std::string typeName() { return valueTypeName(); }
	static std::string typeDescription() { return valueTypeName() + " value"; }
	static const std::type_info& typeInfo() { return typeid(T); }
	static int valueTypeId() { return DataTypeId::getIdOf<value_type>(); }
	static int fullTypeId() { return DataTypeId::getFullTypeOfSingleValue(valueTypeId()); }
	static int size(const value_type& /*v*/) { return 1; }
	static void clear(value_type& v, int /*size*/, bool init) { if(init) v = T(); }
	static const void* getVoidValue(const value_type& v, int /*index*/) { return &v; }
	static void* getVoidValue(value_type& v, int /*index*/) { return &v; }
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

	virtual std::string valueTypeName() const { return value_trait::valueTypeName(); }
	virtual std::string valueTypeNamePlural() const { return value_trait::valueTypeNamePlural(); }
	virtual std::string typeName() const { return value_trait::typeName(); }
	virtual std::string typeDescription() const { return value_trait::typeDescription(); }
	virtual const std::type_info& typeInfo() const { return value_trait::typeInfo(); }
	virtual int valueTypeId() const { return value_trait::valueTypeId(); }
	virtual int fullTypeId() const { return value_trait::fullTypeId(); }
	virtual unsigned int typeColor() const { return value_trait::typeColor(); }

	virtual int size(const void* value) const
	{ return value_trait::size(*static_cast<const value_type*>(value)); }
	virtual void clear(void* value, int size, bool init) const
	{ return value_trait::clear(*static_cast<value_type*>(value), size, init); }
	virtual const void* getVoidValue(const void* value, int index) const
	{ return value_trait::getVoidValue(*static_cast<const value_type*>(value), index); }
	virtual void* getVoidValue(void* value, int index) const
	{ return value_trait::getVoidValue(*static_cast<value_type*>(value), index); }

	virtual void writeValue(XmlElement& elem, const void* value) const
	{ return value_trait::writeValue(elem, *static_cast<const value_type*>(value)); }
	virtual void readValue(const XmlElement& elem, void* value) const
	{ return value_trait::readValue(elem, *static_cast<value_type*>(value)); }
};

//****************************************************************************//

template<class T>
class DataTrait< std::vector<T> >
{
public:
	typedef std::vector<T> vector_type;
	typedef T base_type;
	typedef T value_type;
	typedef DataTrait<base_type> base_trait;

	enum { is_single = 0 };
	enum { is_vector = 1 };
	enum { is_animation = 0 };
	static bool isDisplayed() { return base_trait::isDisplayed(); }
	static bool isPersistent() { return base_trait::isPersistent(); }

	static std::string valueTypeName() { return base_trait::valueTypeName(); }
	static std::string valueTypeNamePlural() { return base_trait::valueTypeNamePlural(); }
	static std::string typeName() { return base_trait::typeName() + "_vector"; }
	static std::string typeDescription() { return "vector of " + valueTypeNamePlural(); }
	static const std::type_info& typeInfo() { return typeid(vector_type); }
	static int valueTypeId() { return DataTypeId::getIdOf<value_type>(); }
	static int fullTypeId() { return DataTypeId::getFullTypeOfVector(valueTypeId()); }
	static unsigned int typeColor() { return base_trait::typeColor(); }
	static int size(const vector_type& v) { return v.size(); }
	static void clear(vector_type& v, int size, bool init)
	{
		if (init)
			v.clear();
		v.resize(size);
	}
	static const void* getVoidValue(const vector_type& vec, int index)
	{
		if (index < 0 || index >= static_cast<int>(vec.size()))
			return nullptr;
		return &vec[index];
	}
	static void* getVoidValue(vector_type& vec, int index)
	{
		if (index < 0 || index >= static_cast<int>(vec.size()))
			return nullptr;
		return &vec[index];
	}
	static void writeValue(XmlElement& elem, const vector_type& vec)
	{
		for (auto& v : vec)
		{
			auto node = elem.addChild("Value");
			base_trait::writeValue(node, v);
		}
	}
	static void readValue(const XmlElement& elem, vector_type& vec)
	{
		vec.clear();
		T t = T();
		for(auto e = elem.firstChild("Value"); e; e = e.nextSibling("Value"))
		{
			base_trait::readValue(e, t);
			vec.push_back(t);
		}
	}
};

//****************************************************************************//

class PANDA_CORE_API DataTraitsList
{
public:
	static AbstractDataTrait* getTrait(int fullTypeId);
	static AbstractDataTrait* getTrait(const std::type_info& type);
	template <class T>
	static AbstractDataTrait* getTraitOf() { return getTrait(typeid(T)); }

private:
	DataTraitsList();

	template<class T> friend class panda::RegisterData;

	static void registerTrait(AbstractDataTrait* trait);
};

} // namespace types

} // namespace panda

#endif // TYPES_DATATRAITS_H
