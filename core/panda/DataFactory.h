#ifndef DATAFACTORY_H
#define DATAFACTORY_H

#include <panda/BaseClass.h>
#include <panda/types/DataTraits.h>
#include <panda/Data.h>
#include <panda/DataCopier.h>

#include <QList>
#include <QSharedPointer>
#include <QMap>

namespace panda
{

class BaseData;
class PandaObject;

class PANDA_CORE_API BaseDataCreator
{
public:
	virtual ~BaseDataCreator() {}
	virtual QSharedPointer<BaseData> create(const QString& name, const QString& help, PandaObject* owner) = 0;
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
		QSharedPointer<BaseDataCreator> creator;
	};

	static DataFactory* getInstance();
	const DataEntry* getEntry(QString className) const;
	const DataEntry* getEntry(int type) const;

	QSharedPointer<BaseData> create(QString className, const QString& name, const QString& help, PandaObject* owner) const;
	QSharedPointer<BaseData> create(int type, const QString& name, const QString& help, PandaObject* owner) const;

	static QString typeToName(int type);
	static int nameToType(QString name);

	typedef QList< QSharedPointer<DataEntry> > EntriesList;
	const EntriesList getEntries() const { return m_entries; }

protected:
	EntriesList m_entries;
	QMap< QString, DataEntry* > m_registry;
	QMap< QString, DataEntry* > m_nameRegistry;
	QMap< int, DataEntry* > m_typeRegistry;
	QMap< int, AbstractDataCopier* > m_copiersMap;

	template<class T> friend class RegisterData;
	void registerData(types::AbstractDataTrait* dataTrait, const BaseClass* theClass, QSharedPointer<BaseDataCreator> creator);

private:
	DataFactory() {}
};

template<class T>
class DataCreator : public BaseDataCreator
{
public:
	virtual QSharedPointer<BaseData> create(const QString& name, const QString& help, PandaObject* owner)
	{
		return QSharedPointer<BaseData>(new T(name, help, owner));
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
												 QSharedPointer< DataCreator<data_type> >::create());
		return 1;
	}
};

} // namespace panda

#endif // OBJECTFACTORY_H
