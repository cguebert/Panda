#include <panda/document/DocumentDatas.h>

#include <panda/helper/algorithm.h>

namespace panda 
{

void DocumentDatas::set(DataList list)
{
	m_dataList = std::move(list);
}

void DocumentDatas::add(const DataPtr& data, int index)
{ 
	if(index < 0 || index >= static_cast<int>(m_dataList.size()))
		m_dataList.push_back(data); 
	else
		m_dataList.insert(m_dataList.begin() + index, data);
}

void DocumentDatas::remove(const DataPtr& data)
{
	auto it = std::find(m_dataList.begin(), m_dataList.end(), data);
	if (it != m_dataList.end())
		m_dataList.erase(it);
}

void DocumentDatas::reorder(const DataPtr& data, int index)
{
	auto it = std::find(m_dataList.begin(), m_dataList.end(), data);
	if (it != m_dataList.end())
		helper::slide(it, it + 1, m_dataList.begin() + index);
	else
		m_dataList.insert(m_dataList.begin() + index, data);
}

} // namespace panda


