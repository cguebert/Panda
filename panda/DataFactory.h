#ifndef DATAFACTORY_H
#define DATAFACTORY_H

#include <panda/BaseClass.h>
#include <panda/DataTraits.h>

#include <QList>
#include <QSharedPointer>
#include <QMap>

namespace panda
{

class BaseData;

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

		QString description;
		QString className;
		int fullType;
		const BaseClass* theClass;
		QSharedPointer<BaseDataCreator> creator;
	};

	static DataFactory* getInstance();
	const DataEntry* getEntry(QString className) const;
	const DataEntry* getEntry(int type) const;

	BaseData* create(QString className, const QString& name, const QString& help, PandaObject* owner);
	BaseData* create(int type, const QString& name, const QString& help, PandaObject* owner);

	static QString typeToDescription(int type);
	static int descriptionToType(QString description);

	typedef QList< QSharedPointer<DataEntry> > EntriesList;
	const EntriesList getEntries() { return entries; }

protected:
	EntriesList entries;
	QMap< QString, DataEntry* > registry;
	QMap< QString, DataEntry* > descriptionRegistry;
	QMap< int, DataEntry* > typeRegistry;

	template<class T> friend class RegisterData;
	void registerData(QString description, int fullType, const BaseClass* theClass, QSharedPointer<BaseDataCreator> creator);

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
		DataFactory::getInstance()->registerData(data_trait<T::value_type>::description(),
												 data_trait<T::value_type>::fullType(),
												 T::getClass(),
												 QSharedPointer<DataCreator<T>>::create());
		return 1;
	}
};

} // namespace panda

#endif // OBJECTFACTORY_H
