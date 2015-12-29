#ifndef BASECLASS_H
#define BASECLASS_H

#include <panda/core.h>

#include <string>
#include <typeinfo>
#include <vector>

namespace panda
{

class DataNode;

class PANDA_CORE_API BaseClass
{
protected:
	BaseClass();
	virtual ~BaseClass();

public:
	const std::string& getTypeName() const;
	const std::string& getNamespaceName() const;
	const std::string& getClassName() const;
	const std::string& getTemplateName() const;

	bool hasParent(const BaseClass* c) const;
	bool operator==(const BaseClass& c) const;
	bool operator!=(const BaseClass& c) const;

	virtual void* dynamicCast(DataNode* obj) const = 0;
	virtual bool isInstance(DataNode* obj) const = 0;

	static std::string decodeTypeName(const std::type_info& type);
	static std::string decodeClassName(const std::type_info& type);
	static std::string decodeNamespaceName(const std::type_info& type);
	static std::string decodeTemplateName(const std::type_info& type);

protected:
	std::vector<const BaseClass*> m_parents;
	std::string m_typeName, m_namespaceName, m_className, m_templateName;
};

#define PANDA_TEMPLATE(Class,P1) Class<P1>
#define PANDA_TEMPLATE2(Class,P1,P2) Class<P1,P2>
#define PANDA_TEMPLATE3(Class,P1,P2,P3) Class<P1,P2,P3>
#define PANDA_TEMPLATE4(Class,P1,P2,P3,P4) Class<P1,P2,P3,P4>

// One of these macros must be used at the beginning of the declaration of a PandaObject
#define PANDA_CLASS(T,Parent) \
	typedef T MyType; \
	typedef ::panda::TClass< T, Parent > MyClass; \
	PANDA_CLASS_DECL

#define PANDA_ABSTRACT_CLASS(T,Parent) \
	typedef T MyType; \
	typedef ::panda::TClass< T, Parent > MyClass; \
	PANDA_ABSTRACT_CLASS_DECL

#define PANDA_CLASS2(T,Parent1,Parent2) \
	typedef T MyType; \
	typedef ::panda::TClass< T, std::pair<Parent1,Parent2> > MyClass; \
	PANDA_CLASS_DECL

#define PANDA_ABSTRACT_CLASS2(T,Parent1,Parent2) \
	typedef T MyType; \
	typedef ::panda::TClass< T, std::pair<Parent1,Parent2> > MyClass; \
	typedef Parent1 Inherit1; \
	typedef Parent2 Inherit2; \
	PANDA_ABSTRACT_CLASS_DECL

#define PANDA_CLASS3(T,Parent1,Parent2,Parent3) \
	typedef T MyType; \
	typedef ::panda::TClass< T, std::pair<Parent1,std::pair<Parent2,Parent3> > > MyClass; \
	PANDA_CLASS_DECL

#define PANDA_ABSTRACT_CLASS3(T,Parent1,Parent2,Parent3) \
	typedef T MyType; \
	typedef ::panda::TClass< T, std::pair<Parent1,std::pair<Parent2,Parent3> > > MyClass; \
	PANDA_ABSTRACT_CLASS_DECL

#define PANDA_CLASS4(T,Parent1,Parent2,Parent3,Parent4) \
	typedef T MyType; \
	typedef ::panda::TClass< T, std::pair<std::pair<Parent1,Parent2>,std::pair<Parent3,Parent4> > > MyClass; \
	PANDA_CLASS_DECL

#define PANDA_ABSTRACT_CLASS4(T,Parent1,Parent2,Parent3,Parent4) \
	typedef T MyType; \
	typedef ::panda::TClass< T, std::pair<std::pair<Parent1,Parent2>,std::pair<Parent3,Parent4> > > MyClass; \
	PANDA_ABSTRACT_CLASS_DECL

#define PANDA_CLASS5(T,Parent1,Parent2,Parent3,Parent4,Parent5) \
	typedef T MyType; \
	typedef ::panda::TClass< T, std::pair<std::pair<Parent1,Parent2>,std::pair<Parent3,std::pair<Parent4,Parent5> > > > MyClass; \
	PANDA_CLASS_DECL

#define PANDA_ABSTRACT_CLASS5(T,Parent1,Parent2,Parent3,Parent4,Parent5) \
	typedef T MyType; \
	typedef ::panda::TClass< T, std::pair<std::pair<Parent1,Parent2>,std::pair<Parent3,std::pair<Parent4,Parent5> > > > MyClass; \
	PANDA_ABSTRACT_CLASS_DECL

// Do not use this macro directly, use PANDA_ABSTRACT_CLASS instead
#define PANDA_ABSTRACT_CLASS_DECL											\
	static const MyClass* GetClass() { return MyClass::getInstance(); }		\
	virtual const ::panda::BaseClass* getClass() const					\
	{ return GetClass(); }													\
	static const char* HeaderFileLocation() { return __FILE__; }

// Do not use this macro directly, use PANDA_CLASS instead
#define PANDA_CLASS_DECL													\
	PANDA_ABSTRACT_CLASS_DECL;

template <class Parents>
class TClassParents
{
public:
	static unsigned int Nb() { return 1; }
	static const BaseClass* GetClass(int i)
	{
		if (i==0)	return Parents::GetClass();
		else		return nullptr;
	}
};

template<>
class TClassParents<void>
{
public:
	static unsigned int Nb() { return 0; }
	static const BaseClass* GetClass(int) { return nullptr; }
};

template<class P1, class P2>
class TClassParents< std::pair<P1,P2> >
{
public:
	static unsigned int Nb() { return TClassParents<P1>::Nb() + TClassParents<P2>::Nb(); }
	static const BaseClass* GetClass(int i)
	{
		if (i<TClassParents<P1>::Nb())	return TClassParents<P1>::GetClass(i);
		else							return TClassParents<P2>::GetClass(i-TClassParents<P1>::Nb());
	}
};

template <class T, class Parents = void>
class TClass : public BaseClass
{
protected:
	TClass()
	{
		m_typeName = decodeTypeName(typeid(T));
		m_namespaceName = decodeNamespaceName(typeid(T));
		m_className = decodeClassName(typeid(T));
		m_templateName = decodeTemplateName(typeid(T));

		unsigned int nb = TClassParents<Parents>::Nb();
		m_parents.reserve(nb);
		for(unsigned int i=0; i<nb; ++i)
			m_parents.push_back(TClassParents<Parents>::GetClass(i));
	}

	virtual ~TClass() {}

	virtual void* dynamicCast(DataNode* obj) const
	{
		return dynamic_cast<T*>(obj);
	}

	virtual bool isInstance(DataNode* obj) const
	{
		return dynamicCast(obj) != nullptr;
	}

public:
	static const TClass<T, Parents>* getInstance()
	{
		static TClass<T, Parents> instance;
		return &instance;
	}
};

} // namespace Panda

#endif // BASECLASS_H
