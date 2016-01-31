#include <panda/object/PandaObject.h>
#include <panda/PandaDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/XmlDocument.h>
#include <panda/helper/algorithm.h>

#include <iostream>

#ifdef PANDA_LOG_EVENTS
#include <panda/UpdateLogger.h>
#endif

namespace panda {

void PandaObject::addData(BaseData* data, int index)
{
	helper::removeAll(m_datas, data);
	
	if(index < 0)
		m_datas.push_back(data);
	else
	{
		index = std::min<int>(index, m_datas.size());
		auto it = m_datas.begin() + index;
		m_datas.insert(it, data);
	}
	emitModified();
}

void PandaObject::removeData(BaseData* data)
{
	if(helper::removeAll(m_datas, data))
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

BaseData* PandaObject::getData(const std::string& name) const
{
	auto iter = std::find_if(m_datas.begin(), m_datas.end(), [name](BaseData* d){
		return d->getName() == name;
	});
	if(iter != m_datas.end())
		return *iter;
	else
		return nullptr;
}

std::vector<BaseData*> PandaObject::getInputDatas() const
{
	std::vector<BaseData*> temp;
	for(BaseData* data : m_datas)
	{
		if(data->isInput())
			temp.push_back(data);
	}

	return temp;
}

std::vector<BaseData*> PandaObject::getOutputDatas() const
{
	std::vector<BaseData*> temp;
	for(BaseData* data : m_datas)
	{
		if(data->isOutput())
			temp.push_back(data);
	}

	return temp;
}

void PandaObject::save(XmlElement& elem, const std::vector<PandaObject*> *selected)
{
	for(BaseData* data : m_datas)
	{
		if(data->isSet() && data->isPersistent() && !data->isReadOnly()
				&& !(selected && data->getParent() && helper::contains(*selected, data->getParent()->getOwner())))
		{
			auto xmlData = elem.addChild("Data");
			xmlData.setAttribute("name", data->getName());
			data->save(xmlData);
		}
	}
}

void PandaObject::load(XmlElement& elem)
{
	auto e = elem.firstChild("Data");
	while(e)
	{
		BaseData* data = getData(e.attribute("name").toString());
		if(data)
			data->load(e);
		e = e.nextSibling("Data");
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


