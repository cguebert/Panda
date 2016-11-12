#ifndef DATAFACTORY_H
#define DATAFACTORY_H

#include <panda/data/BaseClass.h>
#include <panda/data/Data.h>
#include <panda/data/DataCopier.h>
#include <panda/types/DataTraits.h>

#include <map>
#include <memory>

namespace panda
{

class BaseData;
class PandaObject;

class PANDA_CORE_API BaseDataCreator
{
public:
	virtual ~BaseDataCreator() {}
	virtual std::shared_ptr<BaseData> create(const std::string& name, const std::string& help, PandaObject* owner) = 0;
};

class PANDA_CORE_API DataFactory
{
public:
	class DataEntry
	{
	public:
		DataEntry() {}

		std::string typeName;
		std::string className;
		int fullType;
		const BaseClass* theClass;
		std::shared_ptr<BaseDataCreator> creator;
	};

	using EntriesList = std::vector<std::shared_ptr<DataEntry>>;

	static const DataEntry* entry(const std::string& className);
	static const DataEntry* entry(int type);

	static std::shared_ptr<BaseData> create(const std::string& className, const std::string& name, const std::string& help, PandaObject* owner);
	static std::shared_ptr<BaseData> create(int type, const std::string& name, const std::string& help, PandaObject* owner);

	static std::string typeToName(int type);
	static int nameToType(const std::string& name);

	static const EntriesList& entries();

private:
	DataFactory() = default;
	static DataFactory& instance();

	EntriesList m_entries;
	std::map< std::string, DataEntry* > m_registry;
	std::map< std::string, DataEntry* > m_nameRegistry;
	std::map< int, DataEntry* > m_typeRegistry;
	std::map< int, AbstractDataCopier* > m_copiersMap;

	template<class T> friend class RegisterData;
	void registerData(types::AbstractDataTrait* dataTrait, const BaseClass* theClass, std::shared_ptr<BaseDataCreator> creator);
};

template<class T>
class DataCreator : public BaseDataCreator
{
public:
	virtual std::shared_ptr<BaseData> create(const std::string& name, const std::string& help, PandaObject* owner)
	{
		return std::shared_ptr<BaseData>(new T(name, help, owner));
	}
};

template<class T>
class RegisterData
{
public:
	RegisterData() {}
	operator int()
	{
		using value_type = T;
		using data_type = Data<value_type>;
		using data_trait = types::DataTrait<value_type>;

		types::AbstractDataTrait* dataTrait = types::VirtualDataTrait<value_type>::get();
		AbstractDataCopier* dataCopier = VirtualDataCopier<value_type>::get();

		int fullTypeId = dataTrait->fullTypeId();
		types::DataTypeId::registerType<T>(fullTypeId);
		types::DataTraitsList::registerTrait(dataTrait);
		DataCopiersList::registerCopier(fullTypeId, dataCopier);

		DataFactory::instance().registerData(dataTrait,
											 data_type::GetClass(),
											 std::make_shared< DataCreator<data_type> >());
		return 1;
	}
};

} // namespace panda

#endif // OBJECTFACTORY_H
