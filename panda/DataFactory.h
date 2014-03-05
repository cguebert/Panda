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

class BaseDataCreator
{
public:
	virtual ~BaseDataCreator() {}
	virtual BaseData* create(const QString& name, const QString& help, PandaObject* owner) = 0;
};

class DataFactory
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

	BaseData* create(QString className, const QString& name, const QString& help, PandaObject* owner) const;
	BaseData* create(int type, const QString& name, const QString& help, PandaObject* owner) const;

	static QString typeToName(int type);
	static int nameToType(QString name);

	typedef QList< QSharedPointer<DataEntry> > EntriesList;
	const EntriesList getEntries() const { return entries; }

protected:
	EntriesList entries;
	QMap< QString, DataEntry* > registry;
	QMap< QString, DataEntry* > nameRegistry;
	QMap< int, DataEntry* > typeRegistry;

	template<class T> friend class RegisterData;
	void registerData(types::AbstractDataTrait* dataTrait, const BaseClass* theClass, QSharedPointer<BaseDataCreator> creator);

private:
	DataFactory() {}
};

template<class T>
class DataCreator : public BaseDataCreator
{
public:
	virtual BaseData* create(const QString& name, const QString& help, PandaObject* owner)
	{
		return new T(name, help, owner);
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

		data_type::dataTrait = types::VirtualDataTrait<value_type>::get();
		data_type::dataCopier = VirtualDataCopier<value_type>::get();

		types::DataTypeId::registerType<T>(data_type::dataTrait->fullTypeId());

		DataFactory::getInstance()->registerData(data_type::dataTrait,
												 data_type::getClass(),
												 QSharedPointer< DataCreator<data_type> >::create());
		return 1;
	}
};

} // namespace panda

#endif // OBJECTFACTORY_H
