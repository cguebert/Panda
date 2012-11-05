#include <panda/BaseData.h>
#include <panda/PandaObject.h>
#include <iostream>

#include <QApplication>
#include <QVariant>
#include <QMetaType>

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
    , name(init.name)
    , help(init.help)
    , owner(init.owner)
    , parentBaseData(NULL)
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
    , name(name)
    , help(help)
    , owner(owner)
    , parentBaseData(NULL)
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

    return QVariant( (QVariant::Type)parent->getValueType( ))
            .canConvert( (QVariant::Type)this->getValueType() );
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
        isValueSet = true;
        setDirtyValue();
    }
    else
    {
        parentBaseData = NULL;
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
            owner->dataSetParent(this, NULL);
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

QString BaseData::typeToName(int type)
{
    switch(type)
    {
    case QMetaType::Int: 		return "integer";
    case QMetaType::Double:		return "real";
    case QMetaType::QColor:		return "color";
    case QMetaType::QPointF:	return "point";
    case QMetaType::QRectF:		return "rectangle";
    case QMetaType::QString:	return "text";
    case QMetaType::QImage:		return "image";
    default:					return "unknown";
    }
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
	return valueType + (1 << 16);
}

int BaseData::getFullTypeOfVector(int valueType)
{
	return valueType + (1 << 17);
}

int BaseData::getFullTypeOfAnimation(int valueType)
{
	return valueType + (1 << 18);
}

int BaseData::getValueType(int fullType)
{
	return fullType & 0xFFFF;
}

bool BaseData::isSingleValue(int fullType)
{
	return fullType & (1 << 16);
}

bool BaseData::isVector(int fullType)
{
	return fullType & (1 << 17);
}

bool BaseData::isAnimation(int fullType)
{
	return fullType & (1 << 18);
}

int BaseData::replaceValueType(int fullType, int newType)
{
	return (fullType & 0xFFFF0000) + newType;
}

} // namespace panda
