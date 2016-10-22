#ifndef GENERICOBJECT_H
#define GENERICOBJECT_H

#include <panda/object/PandaObject.h>
#include <panda/types/DataTypeId.h>
#include <panda/helper/typeList.h>

#include <functional>
#include <map>

namespace graphview { 
	namespace object {
		class GenericObjectRenderer;
} }

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
		enum DataFlags
		{
			Input = 1 << 0,
			Output = 1 << 1,
			Hidden = 1 << 2, // Not shown in the GUI
			Temporary = 1 << 3 // Not saved
		};

		GenericDataDefinition()
			: type(0), flags(0) {}
		GenericDataDefinition(int type, uint8_t flags, std::string name, std::string help)
			: type(type), flags(flags), name(name), help(help) {}

		bool isInput() const { return (flags & Input) != 0; }
		bool isOutput() const { return (flags & Output) != 0; }
		bool isHidden() const { return (flags & Hidden) != 0; }
		bool isTemporary() const { return (flags & Temporary) != 0; }

		int type;	// Leave the value type part at 0 to use the value type of the connected Data
		uint8_t flags;
		std::string name, help;
	};
	typedef std::vector<GenericDataDefinition> GenericDataDefinitionList;
	typedef std::vector<BaseData*> DataList;

	explicit GenericObject(PandaDocument* parent = nullptr);
	virtual ~GenericObject();

	void update() override;
	void dataSetParent(BaseData* data, BaseData* parent) override;

	void save(XmlElement& elem, const std::vector<PandaObject*>* selected = nullptr) override;
	bool load(const XmlElement& elem) override;

	BaseGenericData* const getGenericData() const; // Access to m_genericData
	virtual BaseData* createDatas(int type, int index = -1);
	virtual void disconnectData(BaseData* data);

protected:
	void doUpdate(bool updateAllInputs = true);
	virtual void reorderDatas();
	virtual void updateDataNames();

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
	friend class graphview::object::GenericObjectRenderer;

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

	void createConnectCommand(BaseData* parent);
	void createDisconnectCommands(const CreatedDatasStructPtr& createdData);
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

inline BaseGenericData* const GenericObject::getGenericData() const
{ return m_genericData; }

inline int GenericObject::nbOfCreatedDatas() const
{ return m_createdDatasStructs.size(); }

inline bool GenericObject::isCreatedData(BaseData* data) const
{ return m_createdDatasMap.count(data) != 0; }

inline void GenericObject::update()
{ doUpdate(); }


//****************************************************************************//

// Works as a GenericObject, but as soon as one input Data is connected,
// it only accepts this type for all future connections.
class PANDA_CORE_API SingleTypeGenericObject : public GenericObject
{
public:
	PANDA_CLASS(SingleTypeGenericObject, GenericObject)

	explicit SingleTypeGenericObject(PandaDocument* parent = nullptr);

	void update() override;
	BaseData* createDatas(int type, int index = -1) override;
	void dataSetParent(BaseData* data, BaseData* parent) override;
	void disconnectData(BaseData* data) override;

protected:
	bool m_singleOutput; // Set this to true to only create outputs for the first connected Data

private:
	int m_connectedType;
};

} // namespace panda

#endif // GENERICOBJECT_H
