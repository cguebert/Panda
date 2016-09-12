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
			addon.addonPtr->save(ObjectAddonNode{ elem, addon.definition });
	}

	void ObjectAddons::load(const XmlElement& elem)
	{
		for (const auto& addon : m_addons)
			addon.addonPtr->load(ObjectAddonNode{ elem, addon.definition });
	}
	
//****************************************************************************//

	ObjectAddonNodeDefinition::ObjectAddonNodeDefinition(const std::string& name, bool hasText)
		: m_name(name)
		, m_hasText(hasText)
	{
	}

	void ObjectAddonNodeDefinition::addAttribute(const std::string& name)
	{
		m_attributes.push_back(name);
	}

	ObjectAddonNodeDefinition& ObjectAddonNodeDefinition::addChild(const std::string& name, bool hasText)
	{
		auto node = std::make_shared<ObjectAddonNodeDefinition>(name, hasText);
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
	const ObjectAddonNodeDefinition::Attributes& ObjectAddonNodeDefinition::attributes() const
	{
		return m_attributes;
	}

	const ObjectAddonNodeDefinition::Nodes& ObjectAddonNodeDefinition::children() const
	{
		return m_children;
	}


//****************************************************************************//

	ObjectAddonNode::ObjectAddonNode(XmlElement element, ObjectAddonNodeDefinition::SPtr definition)
		: m_element(element)
		, m_definition(definition)
	{
	}

	ObjectAddonNode::operator bool() const
	{
		return static_cast<bool>(m_element);
	}

	ObjectAddonNode ObjectAddonNode::addChild(const std::string& name)
	{
		auto childDef = childDefinition(name);
		if (!childDef)
			throw std::exception("Cannot create a child that is not specified in the definition of the addon");

		auto child = m_element.addChild(name);
		return ObjectAddonNode(child, childDef);
	}

	ObjectAddonNode ObjectAddonNode::firstChild(const std::string& name) const
	{
		auto childDef = childDefinition(name);
		if (!childDef)
			throw std::exception("Cannot access a child that is not specified in the definition of the addon");

		auto child = m_element.firstChild(name);
		return ObjectAddonNode(child, childDef);
	}

	ObjectAddonNode ObjectAddonNode::nextSibling(const std::string& name) const
	{
		if (m_definition->name() != name)
			throw std::exception("Cannot access a child that is not specified in the definition of the addon");

		auto child = m_element.nextSibling(name);
		return ObjectAddonNode(child, m_definition);
	}

	XmlAttribute ObjectAddonNode::attribute(const std::string& name) const
	{
		throwIfAttributeIsNotDefined(name);
		return m_element.attribute(name);
	}

	void ObjectAddonNode::setAttribute(const std::string& name, bool value)
	{
		throwIfAttributeIsNotDefined(name);
		m_element.setAttribute(name, value);
	}

	void ObjectAddonNode::setAttribute(const std::string& name, int value)
	{
		throwIfAttributeIsNotDefined(name);
		m_element.setAttribute(name, value);
	}

	void ObjectAddonNode::setAttribute(const std::string& name, unsigned int value)
	{
		throwIfAttributeIsNotDefined(name);
		m_element.setAttribute(name, value);
	}

	void ObjectAddonNode::setAttribute(const std::string& name, float value)
	{
		throwIfAttributeIsNotDefined(name);
		m_element.setAttribute(name, value);
	}

	void ObjectAddonNode::setAttribute(const std::string& name, double value)
	{
		throwIfAttributeIsNotDefined(name);
		m_element.setAttribute(name, value);
	}

	void ObjectAddonNode::setAttribute(const std::string& name, const std::string& value)
	{
		throwIfAttributeIsNotDefined(name);
		m_element.setAttribute(name, value);
	}

	std::string ObjectAddonNode::text() const
	{
		if (!m_definition->hasText())
			throw std::exception("Cannot access a text attribute that is not specified in the definition of the addon");

		return m_element.text();
	}

	void ObjectAddonNode::setText(const std::string& text)
	{
		if (!m_definition->hasText())
			throw std::exception("Cannot access a text attribute that is not specified in the definition of the addon");

		m_element.setText(text);
	}

	void ObjectAddonNode::throwIfAttributeIsNotDefined(const std::string& name) const
	{
		if (!helper::contains(m_definition->attributes(), name))
			throw std::exception("Cannot access an attribute that is not specified in the definition of the addon");
	}

	ObjectAddonNodeDefinition::SPtr ObjectAddonNode::childDefinition(const std::string& name) const
	{
		for (const auto& child : m_definition->children())
		{
			if (child->name() == name)
				return child;
		}

		return nullptr;
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

	void ObjectAddonsRegistry::load(const XmlElement& elem)
	{
		auto node = elem.firstChild("ObjectAddons");
		for (auto addonNode = node.firstChild(); addonNode; addonNode = addonNode.nextSibling())
		{

		}
	}

	const ObjectAddonsRegistry::Addons& ObjectAddonsRegistry::getAddons() const
	{
		return m_addons;
	}

	const ObjectAddonsRegistry::Definitions& ObjectAddonsRegistry::getLoadedDefinitions() const
	{
		return m_loadedDefinitions;
	}

} // namespace Panda


