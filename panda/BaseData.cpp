#include <panda/BaseData.h>
#include <panda/PandaObject.h>
#include <panda/types/DataTraits.h>
#include <panda/types/TypeConverter.h>

#include <QApplication>
#include <iostream>

#include <typeindex>

namespace panda
{

BaseData::BaseData(const BaseInitData& init)
	: m_readOnly(false)
	, m_displayed(true)
	, m_persistent(true)
	, m_input(false)
	, m_output(false)
	, m_isValueSet(false)
	, m_setParentProtection(false)
	, m_counter(0)
	, m_name(init.name)
	, m_help(init.help)
	, m_widget("default")
	, m_owner(init.owner)
	, m_parentBaseData(nullptr)
{
	if(init.data != this)
	{
		std::cerr << "Fatal error : wrong pointer in initData" << std::endl;
		QCoreApplication::exit(-1);
	}

	if(m_owner)
		m_owner->addData(this);
}

BaseData::BaseData(const QString& name, const QString& help, PandaObject* owner)
	: m_readOnly(false)
	, m_displayed(true)
	, m_persistent(true)
	, m_input(false)
	, m_output(false)
	, m_isValueSet(false)
	, m_setParentProtection(false)
	, m_counter(0)
	, m_name(name)
	, m_help(help)
	, m_widget("default")
	, m_owner(owner)
	, m_parentBaseData(nullptr)
{
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
	if(m_setParentProtection)
		return;
	m_setParentProtection = true;

	if(parent)
	{
		/* BUGFIX : only remove non-PandaObjects from inputs
		* This is for the special case of the Group objects
		* where an output Data is connected to another Data
		*/
		for(DataNode* node : m_inputs)
		{
			if(!dynamic_cast<PandaObject*>(node))
				removeInput(node);
		}

		if(parent && !validParent(parent))
			return;

		m_parentBaseData = parent;
		addInput(parent);
		BaseData::setDirtyValue(parent);
//		update();
		++m_counter;
		m_isValueSet = true;
	}
	else
	{
		m_parentBaseData = nullptr;
		while(!m_inputs.empty())
			removeInput(m_inputs.front());
	}

	m_setParentProtection = false;
}

QString BaseData::getDescription() const
{
	return getDataTrait()->typeDescription();
}

void BaseData::save(QDomDocument& doc, QDomElement& elem)
{
	getDataTrait()->writeValue(doc, elem, getVoidValue());
}

void BaseData::load(QDomElement& elem)
{
	auto acc = getVoidAccessor();
	getDataTrait()->readValue(elem, acc);
	QString w = elem.attribute("widget");
	if(!w.isEmpty())
		setWidget(w);

	QString d = elem.attribute("widgetData");
	if(!d.isEmpty())
		setWidgetData(d);
}

void BaseData::doAddInput(DataNode* node)
{
	if(dynamic_cast<PandaObject*>(node))
		m_output = true;
	DataNode::doAddInput(node);
}

void BaseData::doRemoveInput(DataNode* node)
{
	DataNode::doRemoveInput(node);
	if(m_parentBaseData == node)
	{
		if(m_owner && !m_setParentProtection)
			m_owner->dataSetParent(this, nullptr);
	}
	else if(dynamic_cast<PandaObject*>(node))
		m_output = false;
}

void BaseData::doAddOutput(DataNode* node)
{
	if(dynamic_cast<PandaObject*>(node))
		m_input = true;
	DataNode::doAddOutput(node);
}

void BaseData::doRemoveOutput(DataNode* node)
{
	DataNode::doRemoveOutput(node);
	if(dynamic_cast<PandaObject*>(node))
		m_input = false;
}

void BaseData::initFlags()
{
	m_displayed = getDataTrait()->isDisplayed();
	m_persistent = getDataTrait()->isPersistent();
}

//***************************************************************//

VoidDataAccessor::VoidDataAccessor(BaseData* data)
	: m_data(data)
	, m_value(data->beginVoidEdit())
{}

VoidDataAccessor::~VoidDataAccessor()
{ m_data->endVoidEdit(); }

void* VoidDataAccessor::get()
{ return m_value; }

VoidDataAccessor::operator void *()
{ return m_value; }

} // namespace panda
