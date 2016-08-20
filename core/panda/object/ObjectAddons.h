#ifndef OBJECTADDONS_H
#define OBJECTADDONS_H

#include <panda/core.h>

#include <memory>
#include <vector>

namespace panda
{

class PandaObject;
class XmlElement;

class PANDA_CORE_API BaseObjectAddon
{
public:
	using SPtr = std::shared_ptr<BaseObjectAddon>;

	BaseObjectAddon(PandaObject& object);
	virtual ~BaseObjectAddon();

	virtual void save(XmlElement& elem) = 0;
	virtual void load(XmlElement& elem) = 0;

protected:
	PandaObject& m_object;
};

class PANDA_CORE_API ObjectAddons
{
public:
	explicit ObjectAddons(PandaObject& object);

	// Returns null if not present
	template <class T>
	T* get()
	{
		static_assert(std::is_base_of<BaseObjectAddon, T>::value, "The argument of ObjectAddons::get must inherit from BaseObjectAddon");

		for (const auto& addon : m_addons)
		{
			auto ptr = std::dynamic_pointer_cast<T>(addon);
			if (ptr)
				return ptr.get();
		}

		return nullptr;
	}

	// Creates it if not present
	template <class T>
	T& edit()
	{
		static_assert(std::is_base_of<BaseObjectAddon, T>::value, "The argument of ObjectAddons::get must inherit from BaseObjectAddon");

		for (const auto& addon : m_addons)
		{
			auto ptr = std::dynamic_pointer_cast<T>(addon);
			if (ptr)
				return *ptr.get();
		}

		auto addon = std::make_shared<T>(m_object);
		m_addons.push_back(addon);
		return *addon.get();
	}

	void save(XmlElement& elem);
	void load(XmlElement& elem);

private:
	void createAddons();

	PandaObject& m_object;	
	std::vector<BaseObjectAddon::SPtr> m_addons;
};

//****************************************************************************//

class PANDA_CORE_API BaseObjectAddonCreator
{
public:
	using SPtr = std::shared_ptr<BaseObjectAddonCreator>;

	virtual ~BaseObjectAddonCreator() {}
	virtual BaseObjectAddon::SPtr create(PandaObject& object) = 0;
};

template <class T>
class ObjectAddonCreator : public BaseObjectAddonCreator
{
public:
	virtual BaseObjectAddon::SPtr create(PandaObject& object)
	{ return std::make_shared<T>(object); }
};

class PANDA_CORE_API ObjectAddonsRegistry
{
public:
	static ObjectAddonsRegistry& instance();

	template <class T>
	int addObjectAddon()
	{ 
		m_creators.push_back(std::make_shared<ObjectAddonCreator<T>>());
		return 0;
	}

	using Creators = std::vector<BaseObjectAddonCreator::SPtr>;
	const Creators& getCreators() const;
	
private:
	ObjectAddonsRegistry();

	Creators m_creators;
};

template <class T>
class RegisterObjectAddon
{
public:
	RegisterObjectAddon() {}
	operator int()
	{
		ObjectAddonsRegistry::instance().addObjectAddon<T>();
		return 1;
	}
};

} // namespace Panda

#endif // OBJECTADDONS_H
