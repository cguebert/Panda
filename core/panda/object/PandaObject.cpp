#include <panda/object/PandaObject.h>
#include <panda/document/PandaDocument.h>
#include <panda/XmlDocument.h>
#include <panda/helper/algorithm.h>
#include <panda/helper/UpdateLogger.h>
#include <panda/object/ObjectAddons.h>

namespace panda 
{

PandaObject::PandaObject(PandaDocument* document)
	: m_parentDocument(document)
	, m_addons(std::make_unique<ObjectAddons>(*this))
{
}

PandaObject::~PandaObject() = default;

void PandaObject::addData(BaseData* data, int index)
{
	helper::removeAll(m_datas, data);
	
	if(index < 0 || index >= static_cast<int>(m_datas.size()))
		m_datas.push_back(data);
	else
		m_datas.insert(m_datas.begin() + index, data);
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
}

void PandaObject::updateIfDirty() const
{
	if(isDirty() && !m_isUpdating)
	{
		helper::ScopedEvent log(helper::event_update, this);
		m_isUpdating = true;

		auto nonConstThis = const_cast<PandaObject*>(this);
		nonConstThis->update();
		nonConstThis->cleanDirty(); // We force the dirty flag to be cleaned (otherwise the object will not be refreshed if an input changes)

		m_isUpdating = false;
	}
}

void PandaObject::setDirtyValue(const DataNode* caller)
{
	if(!isDirty())
	{
		helper::ScopedEvent log(helper::event_setDirty, this);
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

void PandaObject::load(const XmlElement& elem)
{
	for(auto e = elem.firstChild("Data"); e; e = e.nextSibling("Data"))
	{
		BaseData* data = getData(e.attribute("name").toString());
		if(data)
			data->load(e);
	}
}

void PandaObject::dataSetParent(BaseData* data, BaseData* parent)
{
	data->setParent(parent);
	emitModified();
}

void PandaObject::emitModified()
{
	if (m_doEmitModified && m_parentDocument && !m_destructing)
	{
		m_parentDocument->onModifiedObject(this);
		addons().objectModified();
	}
}

void PandaObject::emitDirty()
{
	if(m_doEmitDirty && !m_isInStep && m_parentDocument && !m_destructing)
		m_parentDocument->onDirtyObject(this);
}

std::string PandaObject::getLabel() const
{
	return "";
}

} // namespace Panda


