#ifndef DATAFACTORY_H
#define DATAFACTORY_H

#include <panda/BaseClass.h>
#include <panda/types/DataTraits.h>
#include <panda/Data.h>
#include <panda/DataCopier.h>

#include <map>
#include <memory>
#include <vector>

namespace panda
{

class BaseData;
class PandaObject;

class PANDA_CORE_API BaseDataCreator
{
public:
	virtual ~BaseDataCreator() {}
	virtual std::shared_ptr<BaseData> create(const QString& name, const QString& help, PandaObject* owner) = 0;
};

class PANDA_CORE_API DataFactory
{
public:
	class DataEntry
	{
	public:
		DataEntry() {}

		QString typeName;
		QString className;
		int fullType;
		const BaseClass* theClass;
		std::shared_ptr<BaseDataCreator> creator;
	};

	static DataFactory* getInstance();
	const DataEntry* getEntry(QString className) const;
	const DataEntry* getEntry(int type) const;

	std::shared_ptr<BaseData> create(QString className, const QString& name, const QString& help, PandaObject* owner) const;
	std::shared_ptr<BaseData> create(int type, const QString& name, const QString& help, PandaObject* owner) const;

	static QString typeToName(int type);
	static int nameToType(QString name);

	typedef std::vector< std::shared_ptr<DataEntry> > EntriesList;
	const EntriesList getEntries() const { return m_entries; }

protected:
	EntriesList m_entries;
	std::map< QString, DataEntry* > m_registry;
	std::map< QString, DataEntry* > m_nameRegistry;
	std::map< int, DataEntry* > m_typeRegistry;
	std::map< int, AbstractDataCopier* > m_copiersMap;

	template<class T> friend class RegisterData;
	void registerData(types::AbstractDataTrait* dataTrait, const BaseClass* theClass, std::shared_ptr<BaseDataCreator> creator);

private:
	DataFactory() {}
};

template<class T>
class DataCreator : public BaseDataCreator
{
public:
	virtual std::shared_ptr<BaseData> create(const QString& name, const QString& help, PandaObject* owner)
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
		typedef T value_type;
		typedef Data<value_type> data_type;
		typedef types::DataTrait<value_type> data_trait;

		types::AbstractDataTrait* dataTrait = types::VirtualDataTrait<value_type>::get();
		AbstractDataCopier* dataCopier = VirtualDataCopier<value_type>::get();

		int fullTypeId = dataTrait->fullTypeId();
		types::DataTypeId::registerType<T>(fullTypeId);
		types::DataTraitsList::registerTrait(dataTrait);
		DataCopiersList::registerCopier(fullTypeId, dataCopier);

		DataFactory::getInstance()->registerData(dataTrait,
												 data_type::GetClass(),
												 std::make_shared< DataCreator<data_type> >());
		return 1;
	}
};

} // namespace panda

#endif // OBJECTFACTORY_H
