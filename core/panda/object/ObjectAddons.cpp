#include <panda/object/ObjectAddons.h>
#include <panda/XmlDocument.h>
#include <panda/helper/algorithm.h>
#include <panda/object/PandaObject.h>

#include <iostream>

namespace panda
{

	BaseObjectAddon::BaseObjectAddon(PandaObject& object)
		: m_object(object)
	{

	}

	BaseObjectAddon::~BaseObjectAddon()
	{

	}

//****************************************************************************//

	ObjectAddons::ObjectAddons(PandaObject& object)
		: m_object(object)
	{
		createAddons();
	}

	void ObjectAddons::createAddons()
	{
		const auto& reg = ObjectAddonsRegistry::instance();
		for (const auto& creator : reg.getCreators())
			m_addons.push_back(creator->create(m_object));
	}

	void ObjectAddons::save(XmlElement& elem)
	{
		for (const auto& addon : m_addons)
			addon->save(elem);
	}

	void ObjectAddons::load(XmlElement& elem)
	{
		for (const auto& addon : m_addons)
			addon->load(elem);
	}

//****************************************************************************//

	ObjectAddonsRegistry& ObjectAddonsRegistry::instance()
	{
		static ObjectAddonsRegistry registry;
		return registry;
	}

	ObjectAddonsRegistry::ObjectAddonsRegistry()
	{

	}

	const ObjectAddonsRegistry::Creators& ObjectAddonsRegistry::getCreators() const
	{
		return m_creators;
	}

} // namespace Panda


