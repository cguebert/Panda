#include <panda/object/ObjectAddons.h>
#include <panda/document/PandaDocument.h>
#include <panda/XmlDocument.h>
#include <panda/helper/algorithm.h>

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
			auto node = elem.addChild("Child");
			node.setAttribute("name", child->name());
			node.setAttribute("hasText", child->hasText());
			saveDefinition(*child, node);
		}
	}

	void loadDefinition(panda::ObjectAddonNodeDefinition& def, const panda::XmlElement& elem)
	{
		for (auto attNode = elem.firstChild("Attribute"); attNode; attNode = attNode.nextSibling("Attribute"))
			def.addAttribute(attNode.attribute("name").toString());

		for (auto childNode = elem.firstChild("Child"); childNode; childNode = childNode.nextSibling("Child"))
		{
			const auto name = childNode.attribute("name").toString();
			const auto hasText = childNode.attribute("hasText").toBool();
			auto& child = def.addChild(name, hasText);
			loadDefinition(child, childNode);
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

		saveNode(elem, m_loadedNode);
	}

	void ObjectAddons::load(const XmlElement& elem)
	{
		for (const auto& addon : m_addons)
			addon.addonPtr->load(ObjectAddonNode{ elem, addon.definition });

		const auto& loadedDefs = ObjectAddonsRegistry::instance().getLoadedDefinitions();
		if (!loadedDefs.empty())
		{
			m_loadedNode.attributes.clear();
			m_loadedNode.children.clear();

			for (const auto& def : loadedDefs)
				loadNode(elem, m_loadedNode, *def);
		}
	}

	void ObjectAddons::saveNode(XmlElement& elem, const TempNode& node)
	{
		for (const auto& attPair : node.attributes)
			elem.setAttribute(attPair.first, attPair.second);

		if (!node.text.empty())
			elem.setText(node.text);

		for (const auto& childNode : node.children)
		{
			auto xmlNode = elem.addChild(childNode->name);
			saveNode(xmlNode, *childNode);
		}
	}

	void ObjectAddons::loadNode(const XmlElement& elem, TempNode& node, const ObjectAddonNodeDefinition& def)
	{
		for (const auto& attName : def.attributes())
		{
			auto val = elem.attribute(attName).toString();
			if (!val.empty())
				node.attributes[attName] = val;
		}

		if (def.hasText())
		{
			auto val = elem.text();
			if (!val.empty())
				node.text = val;
		}

		for (const auto& childDef : def.children())
		{
			const auto childName = childDef->name();
			for (auto childXML = elem.firstChild(childName); childXML; childXML = childXML.nextSibling(childName))
			{
				auto childNode = std::make_shared<TempNode>();
				childNode->name = childName;
				node.children.push_back(childNode);
				loadNode(childXML, *childNode, *childDef);
			}
		}
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

		for (const auto& def : m_loadedDefinitions)
		{
			auto defNode = node.addChild(def->name());
			saveDefinition(*def, defNode);
		}
	}

	void ObjectAddonsRegistry::load(const XmlElement& elem)
	{
		auto node = elem.firstChild("ObjectAddons");
		for (auto addonNode = node.firstChild(); addonNode; addonNode = addonNode.nextSibling())
		{
			auto name = addonNode.name();
			if (helper::contains_if(m_addons, [&name](const AddonInfo& info) {
				return info.definition->name() == name;
			}))
				continue; // Ignore the definitions we already know from the registered addons

			auto def = std::make_shared<ObjectAddonNodeDefinition>(name, false);
			loadDefinition(*def, addonNode);
			m_loadedDefinitions.push_back(def);
		}
	}

	void ObjectAddonsRegistry::clearDefinitions()
	{
		m_loadedDefinitions.clear();
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


