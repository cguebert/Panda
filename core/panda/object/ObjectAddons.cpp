#include <panda/object/ObjectAddons.h>
#include <panda/PandaDocument.h>
#include <panda/XmlDocument.h>
#include <panda/helper/algorithm.h>
#include <panda/object/PandaObject.h>

#include <iostream>

namespace
{

	void saveDefinition(const panda::ObjectAddonNodeDefinition& def, panda::XmlElement& elem)
	{
		for (const auto& attribute : def.attributes())
		{
			auto attNode = elem.addChild("Attribute");
			attNode.setAttribute("name", attribute);
		}

		for (const auto& child : def.children())
		{
			auto node = elem.addChild(child->name());
			node.setAttribute("hasText", child->hasText());
			node.setAttribute("multiplicity", static_cast<int>(child->multiplicity()));
			saveDefinition(*child, node);
		}
	}

}

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
		// Do not create addons for the document
		if (object.parentDocument() && &object != object.parentDocument())
			createAddons();
	}

	void ObjectAddons::createAddons()
	{
		const auto& reg = ObjectAddonsRegistry::instance();
		for (const auto& addonInfo : reg.getAddons())
		{
			auto addonPtr = addonInfo.creator->create(m_object);
			if (addonPtr)
			{
				Addon addon;
				addon.addonPtr = addonPtr;
				addon.definition = addonInfo.definition;
				m_addons.push_back(std::move(addon));
			}
		}
	}

	void ObjectAddons::save(XmlElement& elem)
	{
		for (const auto& addon : m_addons)
			addon.addonPtr->save(elem);
	}

	void ObjectAddons::load(XmlElement& elem)
	{
		for (const auto& addon : m_addons)
			addon.addonPtr->load(elem);
	}
	
//****************************************************************************//

	ObjectAddonNodeDefinition::ObjectAddonNodeDefinition(const std::string& name, bool hasText, NodeMultiplicity multiplicity)
		: m_name(name)
		, m_hasText(hasText)
		, m_multiplicity(multiplicity)
	{
	}

	void ObjectAddonNodeDefinition::addAttribute(const std::string& name)
	{
		m_attributes.push_back(name);
	}

	ObjectAddonNodeDefinition& ObjectAddonNodeDefinition::addChild(const std::string& name, bool hasText, NodeMultiplicity multiplicity)
	{
		auto node = std::make_shared<ObjectAddonNodeDefinition>(name, hasText, multiplicity);
		m_children.push_back(std::move(node));
		return *m_children.back();
	}

	const std::string& ObjectAddonNodeDefinition::name() const
	{
		return m_name;
	}

	bool ObjectAddonNodeDefinition::hasText() const
	{
		return m_hasText;
	}

	ObjectAddonNodeDefinition::NodeMultiplicity ObjectAddonNodeDefinition::multiplicity() const
	{
		return m_multiplicity;
	}

	const ObjectAddonNodeDefinition::Attributes& ObjectAddonNodeDefinition::attributes() const
	{
		return m_attributes;
	}

	const ObjectAddonNodeDefinition::Nodes& ObjectAddonNodeDefinition::children() const
	{
		return m_children;
	}


//****************************************************************************//

	ObjectAddonsRegistry& ObjectAddonsRegistry::instance()
	{
		static ObjectAddonsRegistry registry;
		return registry;
	}

	ObjectAddonsRegistry::ObjectAddonsRegistry() = default;

	void ObjectAddonsRegistry::save(XmlElement& elem)
	{
		auto node = elem.addChild("ObjectAddons");
		for (const auto& addon : m_addons)
		{
			auto def = *addon.definition;
			auto defNode = node.addChild(def.name());
			saveDefinition(def, defNode);
		}
	}

	void ObjectAddonsRegistry::load(XmlElement& elem)
	{
		auto node = elem.firstChild("ObjectAddons");
	}

	const ObjectAddonsRegistry::Addons& ObjectAddonsRegistry::getAddons() const
	{
		return m_addons;
	}

} // namespace Panda


