#include <panda/BaseData.h>
#include <panda/PandaObject.h>
#include <iostream>

#include <QApplication>

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
	if(isAnimation() && !parent->isAnimation())
		return false; // Can not convert to animation, need more parameters

	return (getValueType() == parent->getValueType())		// Either the 2 Datas have the same base type (int & vector of ints for example)
			|| (isNumerical() && parent->isNumerical());	// Or we can convert from one to the other (double & vector of ints)
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

QString BaseData::toString() const
{
	this->updateIfDirty();
	return doToString();
}

QString BaseData::getDescription() const
{
	QString typeName = getValueTypeName();

	if(isSingleValue())
		return QString("Single %1 value").arg(typeName);
	if(isVector())
		return QString("List of %1s").arg(typeName);
	if(isAnimation())
		return QString("Animation of %1s").arg(typeName);

	return QString();
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

int BaseData::getFullType() const
{
	int type = getValueType();
	if(isVector())
		return getFullTypeOfVector(type);
	if(isAnimation())
		return getFullTypeOfAnimation(type);

	return getFullTypeOfSingleValue(type);
}

int BaseData::getFullTypeOfSingleValue(int valueType)
{
	return valueType;
}

int BaseData::getFullTypeOfVector(int valueType)
{
	return valueType + (1 << 16);
}

int BaseData::getFullTypeOfAnimation(int valueType)
{
	return valueType + (1 << 17);
}

int BaseData::getValueType(int fullType)
{
	return fullType & 0xFFFF;
}

bool BaseData::isSingleValue(int fullType)
{
	return !(fullType & 0xFFFF0000);
}

bool BaseData::isVector(int fullType)
{
	return fullType & (1 << 16);
}

bool BaseData::isAnimation(int fullType)
{
	return fullType & (1 << 17);
}

int BaseData::replaceValueType(int fullType, int newType)
{
	return (fullType & 0xFFFF0000) + newType;
}

} // namespace panda
