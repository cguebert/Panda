#include <panda/PandaObject.h>
#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>

#ifdef PANDA_LOG_EVENTS
#include <panda/helper/UpdateLogger.h>
#endif

namespace panda {

const QString dataMarkerStart("~~{");
const QString dataMarkerEnd("}~~");

PandaObject::PandaObject(QObject *parent)
	: QObject(parent)
	, doEmitModified(true)
	, doEmitDirty(true)
	, isUpdating(false)
{
	parentDocument = dynamic_cast<PandaDocument*>(parent);
}

PandaObject::~PandaObject()
{
}

QString PandaObject::getTypeName()
{
	return getClass()->getTypeName();
}

QString PandaObject::getClassName()
{
	return getClass()->getClassName();
}

QString PandaObject::getNamespaceName()
{
	return getClass()->getNamespaceName();
}

QString PandaObject::getTemplateName()
{
	return getClass()->getTemplateName();
}

QString PandaObject::getName() const
{
	return name;
}

quint32 PandaObject::getIndex() const
{
	return index;
}

void PandaObject::addData(BaseData* data)
{
	if(datasMap.contains(data->getName()))
	{
		std::cerr << "Fatal error : another data already have the name " << data->getName().toStdString() << std::endl;
		return;
	}
	if(!datas.contains(data))
	{
		datas.append(data);
		datasMap[data->getName()] = data;
		emitModified();
	}
}

void PandaObject::removeData(BaseData* data)
{
	datasMap.remove(data->getName());
	if(datas.removeAll(data))
		emitModified();
}

void PandaObject::addOutput(BaseData* data)
{
	data->setReadOnly(true);
	DataNode::addOutput(data);
}

void PandaObject::preDestruction()
{	// Some failsafe so the objects being destroyed don't try to update themselves during the operation
	isUpdating = true;
}

void PandaObject::update()
{
	cleanDirty();
}

void PandaObject::updateIfDirty() const
{
	if(isDirty())
	{
		if(!isUpdating)
		{
#ifdef PANDA_LOG_EVENTS
			helper::ScopedEvent log(helper::event_update, this);
#endif
			isUpdating = true;
			const_cast<PandaObject*>(this)->update();
			const_cast<PandaObject*>(this)->cleanDirty();	// Verify if we can remove this call
			isUpdating = false;
		}
	}
}

void PandaObject::setDirtyValue()
{
	if(!dirtyValue)
	{
#ifdef PANDA_LOG_EVENTS
		helper::ScopedEvent log(helper::event_setDirty, this);
#endif
		DataNode::setDirtyValue();
	}

	if(doEmitDirty)
		emit dirty(this);
}

BaseData* PandaObject::getData(const QString& name) const
{
	if(datasMap.contains(name))
		return datasMap[name];
	else return nullptr;
}

QList<BaseData*> PandaObject::getDatas() const
{
	return datas;
}

QList<BaseData*> PandaObject::getInputDatas() const
{
	QList<BaseData*> temp;
	foreach(BaseData* data, datas)
	{
		if(data->isInput())
			temp.append(data);
	}

	return temp;
}

QList<BaseData*> PandaObject::getOutputDatas() const
{
	QList<BaseData*> temp;
	foreach(BaseData* data, datas)
	{
		if(data->isOutput())
			temp.append(data);
	}

	return temp;
}

void PandaObject::setInternalData(const QString& newName, const quint32& newIndex)
{
	name = newName;
	index = newIndex;
}

void PandaObject::save(QDomDocument& doc, QDomElement& elem, const QList<PandaObject*>* selected)
{
	foreach(BaseData* data, datas)
	{
		if(data->isSet() && data->isPersistent() && !data->isReadOnly()
				&& !(selected && data->getParent() && selected->contains(data->getParent()->getOwner())))
		{
			QDomElement xmlData = doc.createElement("Data");
			xmlData.setAttribute("name", data->getName());
			data->save(doc, xmlData);
			elem.appendChild(xmlData);
		}
	}
}

void PandaObject::load(QDomElement& elem)
{
	QDomElement e = elem.firstChildElement("Data");
	while(!e.isNull())
	{
		BaseData* data = getData(e.attribute("name"));
		if(data)
			data->load(e);
		e = e.nextSiblingElement("Data");
	}
}

void PandaObject::dataSetParent(BaseData* data, BaseData* parent)
{
	data->setParent(parent);
	emitModified();
}

void PandaObject::changeDataName(BaseData* data, const QString& newName)
{
	datasMap.remove(data->getName());
	datasMap.insert(newName, data);
}

void PandaObject::emitModified()
{
	if(doEmitModified)
		emit modified(this);
}

} // namespace Panda


