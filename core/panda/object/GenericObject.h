#ifndef GENERICOBJECT_H
#define GENERICOBJECT_H

#include <panda/object/PandaObject.h>
#include <panda/types/DataTypeId.h>
#include <panda/types/Color.h>
#include <panda/types/Gradient.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Polygon.h>
#include <panda/types/Mesh.h>
#include <panda/types/Rect.h>
#include <panda/types/Shader.h>
#include <panda/helper/typeList.h>

#include <functional>
#include <map>

class GenericObjectDrawStruct;

namespace panda
{

class BaseGenericData : public Data<int>
{
public:
	explicit BaseGenericData(const BaseData::BaseInitData& init)
		: Data<int>(init) {}

	virtual bool validParent(const BaseData* parent) const override;
	virtual std::string getTypesName(bool useFullDescription = false) const;
	virtual int getCompatibleType(const BaseData* parent) const;

	std::vector<int> m_allowedTypes;
};

class PANDA_CORE_API GenericData : public BaseGenericData
{
public:
	explicit GenericData(const BaseData::BaseInitData& init)
		: BaseGenericData(init) {}

	virtual std::string getDescription() const override;
};

class PANDA_CORE_API GenericSingleValueData : public BaseGenericData
{
public:
	explicit GenericSingleValueData(const BaseData::BaseInitData& init)
		: BaseGenericData(init) {}

	virtual bool validParent(const BaseData* parent) const override;
	virtual std::string getDescription() const override;
};

class PANDA_CORE_API GenericVectorData : public BaseGenericData
{
public:
	explicit GenericVectorData(const BaseData::BaseInitData& init)
		: BaseGenericData(init) {}

	virtual bool validParent(const BaseData* parent) const override;
	virtual std::string getDescription() const override;
};

class PANDA_CORE_API GenericAnimationData : public BaseGenericData
{
public:
	explicit GenericAnimationData(const BaseData::BaseInitData& init)
		: BaseGenericData(init) {}

	virtual bool validParent(const BaseData* parent) const override;
	virtual std::string getDescription() const override;
};

class PANDA_CORE_API GenericSpecificData : public BaseGenericData	// Accepts only the specific types given allowedTypes
{
public:
	explicit GenericSpecificData(const BaseData::BaseInitData& init)
		: BaseGenericData(init) {}

	virtual bool validParent(const BaseData* parent) const override;
	virtual std::string getDescription() const override;
	virtual int getCompatibleType(const BaseData* parent) const;
};

//****************************************************************************//

class PANDA_CORE_API GenericObject : public PandaObject
{
public:
	PANDA_CLASS(GenericObject, PandaObject)

	struct GenericDataDefinition
	{
		GenericDataDefinition()
			: type(0), input(false), output(false) {}
		GenericDataDefinition(int type, bool input, bool output, std::string name, std::string help)
			: type(type), input(input), output(output), name(name), help(help) {}

		int type;	// Leave the value type part at 0 to use the value type of the connected Data
		bool input, output;
		std::string name, help;
	};
	typedef std::vector<GenericDataDefinition> GenericDataDefinitionList;
	typedef std::vector<BaseData*> DataList;

	explicit GenericObject(PandaDocument* parent = nullptr);
	virtual ~GenericObject();

	virtual void update();
	virtual void dataSetParent(BaseData* data, BaseData* parent);

	virtual void save(XmlElement& elem, const std::vector<PandaObject*>* selected = nullptr);
	virtual void load(XmlElement& elem);

	virtual BaseData* createDatas(int type, int index = -1);

protected:
	void doUpdate(bool updateAllInputs = true);
	virtual void reorderDatas();
	virtual void updateDataNames();

	BaseGenericData* const getGenericData() const; // Access to m_genericData
	int nbOfCreatedDatas() const; // Size of m_createdDatasStructs
	bool isCreatedData(BaseData* data) const; // Return true if data has been created by the GenericObject
	std::vector<int> getRegisteredTypes();
	
	template <class Types, class Parent>
	void setupGenericObject(Parent* ptr, BaseGenericData& data, const GenericDataDefinitionList& defList)
	{
		helper::for_each_type<Types>(functionCreatorWrapper<Parent>(ptr));
		setupGenericData(data, defList);
	}

private:
	friend class SingleTypeGenericObject;
	friend class GenericObjectDrawStruct;

	typedef std::shared_ptr<BaseData> BaseDataPtr;
	typedef std::vector<BaseDataPtr> DataPtrList;

	struct CreatedDatasStruct
	{
		int type;
		DataPtrList datas;
	};

	BaseGenericData* m_genericData;
	GenericDataDefinitionList m_dataDefinitions;

	typedef std::shared_ptr<CreatedDatasStruct> CreatedDatasStructPtr;
	std::vector<CreatedDatasStructPtr> m_createdDatasStructs;
	std::map<BaseData*, CreatedDatasStructPtr> m_createdDatasMap;

	void createUndoCommands(const CreatedDatasStructPtr& createdData);
	void setupGenericData(BaseGenericData& data, const GenericDataDefinitionList& defList);

	using FuncPtr = std::function<void(DataList&)>;
	typedef std::pair<int, FuncPtr> TypeFuncPair;
	std::vector<TypeFuncPair> m_functions;

	template <class T>
	struct functionCreatorWrapper
	{
		functionCreatorWrapper(T* obj) : object(obj) {}
		template<class U> void operator()(U)
		{
			int type = types::DataTypeId::getIdOf<U>();
			auto obj = object;
			object->m_functions.emplace_back(type, [obj](DataList& list) { obj->updateT<U>(list); });
		}

		T* object;
	};

	void invokeFunction(int type, DataList& list);
};

//****************************************************************************//

// Works as a GenericObject, but as soon as one input Data is connected,
// it only accepts this type for all future connections.
class PANDA_CORE_API SingleTypeGenericObject : public GenericObject
{
public:
	PANDA_CLASS(SingleTypeGenericObject, GenericObject)

	explicit SingleTypeGenericObject(PandaDocument* parent = nullptr);

	virtual void update();
	virtual BaseData* createDatas(int type, int index = -1);
	virtual void dataSetParent(BaseData* data, BaseData* parent);

protected:
	bool m_singleOutput; // Set this to true to only create outputs for the first connected Data

private:
	int m_connectedType;
};

//****************************************************************************//

typedef std::tuple<int, PReal, types::Color, types::Point, types::Rect, std::string, types::ImageWrapper,
	types::Gradient, types::Shader, types::Mesh, types::Path, types::Polygon> allDataTypes;
typedef std::tuple<int, PReal, types::Color, types::Point, types::Rect, std::string> allSearchableTypes;
typedef std::tuple<int, PReal, types::Color, types::Point, types::Rect> allNumericalTypes;
typedef std::tuple<PReal, types::Color, types::Point, types::Gradient> allAnimationTypes;

} // namespace panda

#endif // GENERICOBJECT_H
