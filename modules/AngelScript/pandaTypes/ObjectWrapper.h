#pragma once

#include <panda/object/PandaObject.h>
#include "DataWrapper.h"

class asIScriptEngine;

namespace panda
{

struct DataInfo
{
	bool input = true;
	std::shared_ptr<BaseData> data;
	std::shared_ptr<BaseDataWrapper> dataWrapper;
};

class ObjectWrapper
{
public:
	ObjectWrapper(PandaObject* object);

	// For the scripts
	template <class T>
	BaseDataWrapper* createData(bool input, const std::string& name, const std::string& help)
	{
		auto data = std::make_shared<Data<T>>(name, help, m_object);
		auto dataWrapper = std::make_shared<DataWrapper<T>>(data.get());

		DataInfo info;
		info.input = input;
		info.data = data;
		info.dataWrapper = dataWrapper;
		m_datas.push_back(info);

		return dataWrapper.get();
	}

	// For the AS_Object
	void clear();
	const std::vector<DataInfo>& datas();
	
private:
	PandaObject* m_object = nullptr;
	std::vector<DataInfo> m_datas;
};

inline ObjectWrapper::ObjectWrapper(PandaObject* object)
	: m_object(object)
{ }

inline void ObjectWrapper::clear()
{ m_datas.clear(); }

inline const std::vector<DataInfo>& ObjectWrapper::datas()
{ return m_datas; }

} // namespace panda
