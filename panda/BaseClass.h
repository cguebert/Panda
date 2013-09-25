#ifndef BASECLASS_H
#define BASECLASS_H

#include <QString>
#include <QList>
#include <typeinfo>

namespace panda
{

class DataNode;

class BaseClass
{
protected:
	BaseClass();
	virtual ~BaseClass();

public:
	QString getTypeName() const;
	QString getNamespaceName() const;
	QString getClassName() const;
	QString getTemplateName() const;

	bool hasParent(const BaseClass* c) const;
	bool operator==(const BaseClass& c) const;
	bool operator!=(const BaseClass& c) const;

	virtual void* dynamicCast(DataNode* obj) const = 0;
	virtual bool isInstance(DataNode* obj) const = 0;

	static QString decodeTypeName(const std::type_info& type);
	static QString decodeClassName(const std::type_info& type);
	static QString decodeNamespaceName(const std::type_info& type);
	static QString decodeTemplateName(const std::type_info& type);

protected:
	QList<const BaseClass*> parents;
	QString typeName, namespaceName, className, templateName;
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
	static const MyClass* getClass() { return MyClass::getInstance(); }		\
	virtual const ::panda::BaseClass* getBaseClass() const					\
	{ return getClass(); }													\
	static const char* HeaderFileLocation() { return __FILE__; }

// Do not use this macro directly, use PANDA_CLASS instead
#define PANDA_CLASS_DECL                                        \
	PANDA_ABSTRACT_CLASS_DECL;

template <class Parents>
class TClassParents
{
public:
	static unsigned int nb() { return 1; }
	static const BaseClass* getClass(int i)
	{
		if (i==0)	return Parents::getClass();
		else		return nullptr;
	}
};

template<>
class TClassParents<void>
{
public:
	static unsigned int nb() { return 0; }
	static const BaseClass* getClass(int) { return nullptr; }
};

template<class P1, class P2>
class TClassParents< std::pair<P1,P2> >
{
public:
	static unsigned int nb() { return TClassParents<P1>::nb() + TClassParents<P2>::nb(); }
	static const BaseClass* getClass(int i)
	{
		if (i<TClassParents<P1>::nb())	return TClassParents<P1>::getClass(i);
		else							return TClassParents<P2>::getClass(i-TClassParents<P1>::nb());
	}
};

template <class T, class Parents = void>
class TClass : public BaseClass
{
protected:
	TClass()
	{
		typeName = decodeTypeName(typeid(T));
		namespaceName = decodeNamespaceName(typeid(T));
		className = decodeClassName(typeid(T));
		templateName = decodeTemplateName(typeid(T));

		unsigned int nb = TClassParents<Parents>::nb();
		parents.reserve(nb);
		for(unsigned int i=0; i<nb; ++i)
			parents.push_back(TClassParents<Parents>::getClass(i));
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
