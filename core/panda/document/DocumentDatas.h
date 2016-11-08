#pragma once

#include <panda/data/BaseData.h>

#include <memory>

namespace panda {

class PANDA_CORE_API DocumentDatas
{
public:
	using DataPtr = std::shared_ptr<BaseData>;
	using DataList = std::vector<DataPtr>;

	const DataList& get() const;
	void set(DataList list);

	void add(const DataPtr& data, int index = -1);
	void remove(const DataPtr& data);
	void reorder(const DataPtr& data, int index);

protected:
	DataList m_dataList;
};

inline const DocumentDatas::DataList& DocumentDatas::get() const
{ return m_dataList; }

} // namespace panda

