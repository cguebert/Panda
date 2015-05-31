#include <panda/PandaObject.h>
#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>

#include <iostream>

#ifdef PANDA_LOG_EVENTS
#include <panda/helper/UpdateLogger.h>
#endif

#include <QDomDocument>

namespace panda {

PandaObject::PandaObject(PandaDocument* document)
	: m_index(0)
	, m_parentDocument(document)
	, m_doEmitModified(true)
	, m_doEmitDirty(true)
	, m_isInStep(false)
	, m_isUpdating(false)
	, m_laterUpdate(false)
	, m_destructing(false)
{
}

void PandaObject::addData(BaseData* data)
{
	if(getData(data->getName()))
	{
		std::cerr << "Fatal error : another data already have the name " << data->getName().toStdString() << std::endl;
		return;
	}
	if(!m_datas.contains(data))
	{
		m_datas.push_back(data);
		emitModified();
	}
}

void PandaObject::removeData(BaseData* data)
{
	if(m_datas.removeAll(data))
		emitModified();
}

void PandaObject::addOutput(BaseData& data)
{
	data.setReadOnly(true);
	DataNode::addOutput(data);
}

void PandaObject::postCreate()
{
	for(auto data : m_datas)
		data->unset();
}

void PandaObject::preDestruction()
{	// Some failsafe so the objects being destroyed don't try to update themselves during the operation
	m_isUpdating = true;
	m_destructing = true;
}

void PandaObject::update()
{
	cleanDirty();
}

void PandaObject::updateIfDirty() const
{
	if(isDirty() && !m_isUpdating)
	{
#ifdef PANDA_LOG_EVENTS
		helper::ScopedEvent log(helper::event_update, this);
#endif
		m_isUpdating = true;
		const_cast<PandaObject*>(this)->update();
		const_cast<PandaObject*>(this)->cleanDirty();	// Verify if we can remove this call
		m_isUpdating = false;
	}
}

void PandaObject::setDirtyValue(const DataNode* caller)
{
	if(!isDirty())
	{
#ifdef PANDA_LOG_EVENTS
		helper::ScopedEvent log(helper::event_setDirty, this);
#endif
		DataNode::setDirtyValue(caller);
	}

	emitDirty();
}

BaseData* PandaObject::getData(const QString& name) const
{
	auto iter = std::find_if(m_datas.begin(), m_datas.end(), [name](BaseData* d){
		return d->getName() == name;
	});
	if(iter != m_datas.end())
		return *iter;
	else
		return nullptr;
}

QList<BaseData*> PandaObject::getInputDatas() const
{
	QList<BaseData*> temp;
	for(BaseData* data : m_datas)
	{
		if(data->isInput())
			temp.push_back(data);
	}

	return temp;
}

QList<BaseData*> PandaObject::getOutputDatas() const
{
	QList<BaseData*> temp;
	for(BaseData* data : m_datas)
	{
		if(data->isOutput())
			temp.push_back(data);
	}

	return temp;
}

void PandaObject::save(QDomDocument& doc, QDomElement& elem, const QList<PandaObject*> *selected)
{
	for(BaseData* data : m_datas)
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

void PandaObject::emitModified()
{
	if(m_doEmitModified && m_parentDocument && !m_destructing)
		m_parentDocument->onModifiedObject(this);
}

void PandaObject::emitDirty()
{
	if(m_doEmitDirty && !m_isInStep && m_parentDocument && !m_destructing)
		m_parentDocument->onDirtyObject(this);
}

} // namespace Panda


