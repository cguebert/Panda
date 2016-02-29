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
	BaseDataWrapper* createIntData(bool input, const std::string& name, const std::string& help);
	BaseDataWrapper* createFloatData(bool input, const std::string& name, const std::string& help);
	BaseDataWrapper* createPointData(bool input, const std::string& name, const std::string& help);
	BaseDataWrapper* createRectData(bool input, const std::string& name, const std::string& help);
	BaseDataWrapper* createColorData(bool input, const std::string& name, const std::string& help);

	// For the AS_Object
	void clear();
	const std::vector<DataInfo>& datas();

	static void registerObject(asIScriptEngine* engine);
	
private:
	template <class T>
	BaseDataWrapper* createData(bool input, const std::string& name, const std::string& help);

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
