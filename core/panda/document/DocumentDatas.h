#pragma once

#include <panda/data/BaseData.h>

#include <memory>

namespace panda 
{

class PandaObject;

std::string findAvailableDataName(PandaObject* parent, const std::string& baseName, BaseData* data = nullptr);
std::shared_ptr<BaseData> duplicateData(PandaObject* parent, BaseData* data);

class PANDA_CORE_API DocumentDatas
{
public:
	using DataPtr = std::shared_ptr<BaseData>;
	using DataList = std::vector<DataPtr>;

	DocumentDatas(PandaObject* parent);

	const DataList& get() const;
	void set(DataList list);

	void add(const DataPtr& data, int index = -1);
	void remove(const DataPtr& data);
	void reorder(const DataPtr& data, int index);

	void save(XmlElement& elem, const std::string& nodeName);
	void load(const XmlElement& elem, const std::string& nodeName);

	PandaObject* parent() const;

protected:
	DataList m_dataList;
	PandaObject* m_parent;
};

inline const DocumentDatas::DataList& DocumentDatas::get() const
{ return m_dataList; }

inline PandaObject* DocumentDatas::parent() const
{ return m_parent; }

} // namespace panda

