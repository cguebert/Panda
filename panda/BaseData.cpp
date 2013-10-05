#include <panda/BaseData.h>
#include <panda/PandaObject.h>
#include <panda/DataTraits.h>

#include <QApplication>
#include <iostream>

#include <typeindex>

namespace panda
{

BaseData::BaseData(const BaseInitData& init)
	: readOnly(false)
	, displayed(true)
	, persistent(true)
	, input(false)
	, output(false)
	, isValueSet(false)
	, setParentProtection(false)
	, counter(0)
	, name(init.name)
	, help(init.help)
	, widget("default")
	, owner(init.owner)
	, parentBaseData(nullptr)
{
	if(init.data != this)
	{
		std::cerr << "Fatal error : wrong pointer in initData" << std::endl;
		QCoreApplication::exit(-1);
	}

	if(owner)
		owner->addData(this);
}

BaseData::BaseData(const QString& name, const QString& help, PandaObject* owner)
	: readOnly(false)
	, displayed(true)
	, persistent(true)
	, input(false)
	, output(false)
	, isValueSet(false)
	, setParentProtection(false)
	, counter(0)
	, name(name)
	, help(help)
	, widget("default")
	, owner(owner)
	, parentBaseData(nullptr)
{
	if(owner)
		owner->addData(this);
}

void BaseData::setName(const QString& newName)
{
	if(owner)
		owner->changeDataName(this, newName);
	name = newName;
}

bool BaseData::validParent(const BaseData* parent) const
{
	auto trait = getDataTrait();
	auto parentTrait = parent->getDataTrait();
	if(trait->isAnimation() && !parentTrait->isAnimation())
		return false; // Can not convert to animation, need more parameters

	return (trait->valueTypeId() == parentTrait->valueTypeId())		// Either the 2 Datas have the same base type (int & vector of ints for example)
			|| (trait->isNumerical() && parentTrait->isNumerical());	// Or we can convert from one to the other (double & vector of ints)
}

void BaseData::setParent(BaseData* parent)
{
	if(parentBaseData == parent)
		return;
	if(setParentProtection)
		return;
	setParentProtection = true;

	if(parent)
	{
		/* BUGFIX : only remove non-PandaObjects from inputs
		* This is for the special case of the Group objects
		* where an output Data is connected to another Data
		*/
		foreach(DataNode* node, this->inputs)
		{
			if(!dynamic_cast<PandaObject*>(node))
				this->removeInput(node);
		}

		if(parent && !validParent(parent))
			return;

		parentBaseData = parent;
		addInput(parent);
		BaseData::setDirtyValue();
		update();
		++counter;
		isValueSet = true;
		setDirtyValue();
	}
	else
	{
		parentBaseData = nullptr;
		while(!this->inputs.empty())
			this->removeInput(this->inputs.front());
	}

	setParentProtection = false;
}

void BaseData::update()
{
	cleanDirty();
	foreach(DataNode* node, inputs)
		node->updateIfDirty();
	if(parentBaseData)
		copyValueFrom(parentBaseData);
}

QString BaseData::getDescription() const
{
	auto trait = getDataTrait();
	if(trait->isSingleValue())
		return QString("Single %1 value").arg(trait->valueTypeName());
	if(trait->isVector())
		return QString("List of %1").arg(trait->valueTypeNamePlural());
	if(trait->isAnimation())
		return QString("Animation of %1").arg(trait->valueTypeNamePlural());

	return QString();
}

void BaseData::save(QDomDocument& doc, QDomElement& elem)
{
	getDataTrait()->writeValue(doc, elem, getVoidValue());
}

void BaseData::load(QDomElement& elem)
{
	auto acc = getVoidAccessor();
	getDataTrait()->readValue(elem, acc);
}

void BaseData::doAddInput(DataNode* node)
{
	if(dynamic_cast<PandaObject*>(node))
		output = true;
	DataNode::doAddInput(node);
}

void BaseData::doRemoveInput(DataNode* node)
{
	DataNode::doRemoveInput(node);
	if(parentBaseData == node)
	{
		if(owner && !setParentProtection)
			owner->dataSetParent(this, nullptr);
	}
	else if(dynamic_cast<PandaObject*>(node))
		output = false;
}

void BaseData::doAddOutput(DataNode* node)
{
	if(dynamic_cast<PandaObject*>(node))
		input = true;
	DataNode::doAddOutput(node);
}

void BaseData::doRemoveOutput(DataNode* node)
{
	DataNode::doRemoveOutput(node);
	if(dynamic_cast<PandaObject*>(node))
		input = false;
}

void BaseData::initFlags()
{
	displayed = getDataTrait()->isDisplayed();
	persistent = getDataTrait()->isPersistent();
}

//***************************************************************//

VoidDataAccessor::VoidDataAccessor(BaseData* d)
	: data(d)
	, value(d->beginVoidEdit())
{}

VoidDataAccessor::~VoidDataAccessor()
{ data->endVoidEdit(); }

void* VoidDataAccessor::get()
{ return value; }

VoidDataAccessor::operator void *()
{ return value; }

} // namespace panda
