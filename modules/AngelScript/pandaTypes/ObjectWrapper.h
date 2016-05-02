#pragma once

#include <panda/object/PandaObject.h>
#include "DataWrapper.h"

class asIScriptEngine;

namespace panda
{

void registerObject(asIScriptEngine* engine);

class ObjectWrapper
{
public:
	struct DataInfo
	{
		bool input = false, output = false;
		std::shared_ptr<BaseData> data;
		std::shared_ptr<BaseDataWrapper> dataWrapper;
	};

	ObjectWrapper(PandaObject* object, asIScriptEngine* engine);

	// For the scripts
	template <class T, class Wrapper = DataWrapper<T>>
	BaseDataWrapper* createData(const std::string& name, const std::string& help)
	{
		auto data = std::make_shared<Data<T>>(name, help, m_object);
		auto dataWrapper = std::make_shared<Wrapper>(data.get(), m_engine);

		DataInfo info;
		info.data = data;
		info.dataWrapper = dataWrapper;
		m_datas.push_back(info);

		return dataWrapper.get();
	}

	void setInput(BaseDataWrapper* wrapper);
	void setOutput(BaseDataWrapper* wrapper);

	// For the AS_Object
	void clear();
	const std::vector<DataInfo>& datas();
	
private:
	PandaObject* m_object = nullptr;
	asIScriptEngine* m_engine = nullptr;
	std::vector<DataInfo> m_datas;
};

inline ObjectWrapper::ObjectWrapper(PandaObject* object, asIScriptEngine* engine)
	: m_object(object), m_engine(engine)
{ }

inline void ObjectWrapper::clear()
{ m_datas.clear(); }

inline const std::vector<ObjectWrapper::DataInfo>& ObjectWrapper::datas()
{ return m_datas; }

} // namespace panda
