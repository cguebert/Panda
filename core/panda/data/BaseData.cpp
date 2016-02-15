#include <panda/data/BaseData.h>
#include <panda/data/DataCopier.h>
#include <panda/object/PandaObject.h>
#include <panda/types/DataTraits.h>
#include <panda/types/TypeConverter.h>
#include <panda/UpdateLogger.h>

#include <iostream>
#include <typeindex>

namespace panda
{

BaseData::BaseData(const BaseInitData& init, const std::type_info& type)
	: m_dataFlags(FLAG_DEFAULT)
	, m_counter(0)
	, m_name(init.name)
	, m_help(init.help)
	, m_widget("default")
	, m_owner(init.owner)
	, m_parentBaseData(nullptr)
{
	initInternals(type);

	if(m_owner)
		m_owner->addData(this);
}

BaseData::BaseData(const std::string& name, const std::string& help, PandaObject* owner, const std::type_info& type)
	: m_dataFlags(FLAG_DEFAULT)
	, m_counter(0)
	, m_name(name)
	, m_help(help)
	, m_widget("default")
	, m_owner(owner)
	, m_parentBaseData(nullptr)
{
	initInternals(type);

	if(m_owner)
		m_owner->addData(this);
}

bool BaseData::validParent(const BaseData* parent) const
{
	auto trait = getDataTrait();
	auto parentTrait = parent->getDataTrait();
	if(trait->isAnimation() != parentTrait->isAnimation())
		return false; // Can not convert to or from animation

	using types::TypeConverter;
	int parentValueType = parentTrait->valueTypeId(), parentFullType = parentTrait->fullTypeId();
	int valueType = trait->valueTypeId(), fullType = trait->fullTypeId();
	return (fullType == parentFullType)		// Either the 2 Datas have the same type
		|| (valueType == parentValueType)	// Or the same base type
		|| TypeConverter::canConvert(parentFullType, fullType)	// Or we have a direct converter from a type to the other
		|| TypeConverter::canConvert(parentValueType, valueType)// Or the base types are convertible
		|| TypeConverter::canConvert(parentValueType, fullType)	// Or we can convert one type to the other's value type
		|| TypeConverter::canConvert(parentFullType, valueType);// Or vice-versa
}

void BaseData::setParent(BaseData* parent)
{
	if(m_parentBaseData == parent)
		return;
	if(getFlag(FLAG_SETPARENTPROTECTION))
		return;
	setFlag(FLAG_SETPARENTPROTECTION, true);

	if(parent)
	{
		/* BUGFIX : only remove non-PandaObjects from inputs
		* This is for the special case of the Group objects
		* where an output Data is connected to another Data
		*/
		auto inputs = m_inputs; // Iterate over a copy
		for(DataNode* node : inputs)
		{
			if(!dynamic_cast<PandaObject*>(node))
				removeInput(*node);
		}

		if(parent && !validParent(parent))
			return;

		m_parentBaseData = parent;
		addInput(*parent);
		BaseData::setDirtyValue(parent);
		++m_counter;
		forceSet();
	}
	else
	{
		m_parentBaseData = nullptr;
		while(!m_inputs.empty())
			removeInput(*m_inputs.front());
	}

	setFlag(FLAG_SETPARENTPROTECTION, false);
}

std::string BaseData::getDescription() const
{
	return getDataTrait()->typeDescription();
}

void BaseData::copyValueFrom(const BaseData* from)
{
	helper::ScopedEvent log(helper::event_copyValue, this);
	if(m_dataCopier->copyData(this, from))
		forceSet();
}

void BaseData::save(XmlElement& elem) const
{
	getDataTrait()->writeValue(elem, getVoidValue());
}

void BaseData::load(XmlElement& elem)
{
	auto acc = getVoidAccessor();
	getDataTrait()->readValue(elem, acc);
	std::string w = elem.attribute("widget").toString();
	if(!w.empty())
		setWidget(w);

	std::string d = elem.attribute("widgetData").toString();
	if(!d.empty())
		setWidgetData(d);
}

void BaseData::doAddInput(DataNode& node)
{
	if(dynamic_cast<PandaObject*>(&node))
		setOutput(true);
	DataNode::doAddInput(node);
}

void BaseData::doRemoveInput(DataNode& node)
{
	DataNode::doRemoveInput(node);
	if(m_parentBaseData == &node)
	{
		if(m_owner && !getFlag(FLAG_SETPARENTPROTECTION))
			m_owner->dataSetParent(this, nullptr);
	}
	else if(dynamic_cast<PandaObject*>(&node))
		setOutput(false);
}

void BaseData::doAddOutput(DataNode& node)
{
	if(dynamic_cast<PandaObject*>(&node))
		setInput(true);
	DataNode::doAddOutput(node);
}

void BaseData::doRemoveOutput(DataNode& node)
{
	DataNode::doRemoveOutput(node);
	if(dynamic_cast<PandaObject*>(&node))
		setInput(false);
}

void BaseData::initInternals(const std::type_info& type)
{
	m_dataTrait = types::DataTraitsList::getTrait(type);
	m_dataCopier = DataCopiersList::getCopier(type);

	setFlag(FLAG_DISPLAYED, getDataTrait()->isDisplayed());
	setFlag(FLAG_PERSISTENT, getDataTrait()->isPersistent());
}

void BaseData::setDirtyValue(const DataNode* caller)
{
	if(!isDirty())
	{
		helper::ScopedEvent log(helper::event_setDirty, this);
		DataNode::setDirtyValue(caller);
	}
}

} // namespace panda
