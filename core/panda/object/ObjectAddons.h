#ifndef OBJECTADDONS_H
#define OBJECTADDONS_H

#include <panda/data/BaseClass.h>

#include <memory>
#include <string>
#include <vector>

namespace panda
{

class ObjectAddonNodeDefinition;
class PandaObject;
class XmlAttribute;
class XmlElement;

class PANDA_CORE_API BaseObjectAddon
{
public:
	using SPtr = std::shared_ptr<BaseObjectAddon>;

	BaseObjectAddon(PandaObject& object);
	virtual ~BaseObjectAddon();

	// Each Addon class must define the following method:
//	static void setDefinition(ObjectAddonNodeDefinition& nodeDefinition);

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
			auto ptr = std::dynamic_pointer_cast<T>(addon.addonPtr);
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
			auto ptr = std::dynamic_pointer_cast<T>(addon.addonPtr);
			if (ptr)
				return *ptr;
		}

		auto ptr = std::make_shared<T>(m_object);
		auto def = ObjectAddonsRegistry::instance().getDefinition<T>();
		if (!def)
			throw std::exception("Could not find the addon definition for this type");

		Addon addon;
		addon.addonPtr = ptr;
		addon.definition = def;
		m_addons.push_back(addon);
		return *ptr;
	}

	void save(XmlElement& elem);
	void load(XmlElement& elem);

private:
	void createAddons();

	PandaObject& m_object;

	struct Addon
	{
		BaseObjectAddon::SPtr addonPtr;
		std::shared_ptr<ObjectAddonNodeDefinition> definition;
	};
	std::vector<Addon> m_addons;
};

//****************************************************************************//

class PANDA_CORE_API ObjectAddonNodeDefinition
{
public:
	enum class NodeMultiplicity { Single, Multiple };
	using Attributes = std::vector<std::string>;
	using SPtr = std::shared_ptr<ObjectAddonNodeDefinition>;
	using Nodes = std::vector<SPtr>;

	ObjectAddonNodeDefinition(const std::string& name, bool hasText, NodeMultiplicity multiplicity);

	// Creation of the definition
	void addAttribute(const std::string& name);
	ObjectAddonNodeDefinition& addChild(const std::string& name, bool hasText, NodeMultiplicity multiplicity);

	// Access to the definition
	const std::string& name() const;
	bool hasText() const;
	NodeMultiplicity multiplicity() const;
	const Attributes& attributes() const;
	const Nodes& children() const;

private:
	std::string m_name;
	bool m_hasText = false;
	NodeMultiplicity m_multiplicity = NodeMultiplicity::Single;
	Attributes m_attributes;
	Nodes m_children;
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
	struct AddonInfo
	{
		BaseObjectAddonCreator::SPtr creator;
		ObjectAddonNodeDefinition::SPtr definition;
	};
	using Addons = std::vector<AddonInfo>;

	static ObjectAddonsRegistry& instance();

	void save(XmlElement& elem);
	void load(XmlElement& elem);

	template <class T>
	void addObjectAddon()
	{ 
		auto name = BaseClass::decodeTypeName(typeid(T));
		auto def = std::make_shared<ObjectAddonNodeDefinition>(name, false, ObjectAddonNodeDefinition::NodeMultiplicity::Single);
		T::setDefinition(*def);

		AddonInfo info;
		info.creator = std::make_shared<ObjectAddonCreator<T>>();
		info.definition = def;
		m_addons.push_back(std::move(info));
	}
	
	const Addons& getAddons() const;

	template <class T>
	ObjectAddonNodeDefinition::SPtr getDefinition() const
	{
		for (const auto& addon : m_addons)
		{
			if (std::dynamic_pointer_cast<ObjectAddonCreator<T>>(addon.creator))
				return addon.definition;
		}

		return nullptr;
	}
	
private:
	ObjectAddonsRegistry();

	Addons m_addons;
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
