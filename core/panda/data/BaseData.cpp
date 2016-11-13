#include <panda/data/BaseData.h>
#include <panda/data/DataCopier.h>
#include <panda/object/PandaObject.h>
#include <panda/types/DataTraits.h>
#include <panda/types/TypeConverter.h>
#include <panda/helper/algorithm.h>
#include <panda/helper/UpdateLogger.h>

#include <iostream>
#include <typeindex>

namespace panda
{

BaseData::BaseData(const BaseInitData& init, const std::type_info& type)
	: m_name(init.name)
	, m_help(init.help)
	, m_widget("default")
	, m_owner(init.owner)
{
	initInternals(type);

	if(m_owner)
		m_owner->addData(this);
}

BaseData::BaseData(const std::string& name, const std::string& help, PandaObject* owner, const std::type_info& type)
	: m_name(name)
	, m_help(help)
	, m_widget("default")
	, m_owner(owner)
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
	if(getFlag(DataOption::SetParentProtection))
		return;
	setFlag(DataOption::SetParentProtection, true);

	if (m_parentBaseData)
		removeInput(*m_parentBaseData);

	if(parent)
	{
		if(!validParent(parent))
			return;

		m_parentBaseData = parent;
		addInput(*parent);
		BaseData::setDirtyValue(parent);
		++m_counter;
		forceSet();
	}
	else
		m_parentBaseData = nullptr;

	setFlag(DataOption::SetParentProtection, false);
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

void BaseData::load(const XmlElement& elem)
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
		if(m_owner && !getFlag(DataOption::SetParentProtection))
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

	setFlag(DataOption::Displayed, getDataTrait()->isDisplayed());
	setFlag(DataOption::Persistent, getDataTrait()->isPersistent());
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
