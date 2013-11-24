#ifndef DATAWIDGETFACTORY_H
#define DATAWIDGETFACTORY_H

#include <panda/types/DataTraits.h>

#include <QSharedPointer>
#include <QMap>

class QWidget;
class BaseDataWidget;

namespace panda
{
class BaseData;
template <class T> class Data;
}

class BaseDataWidgetCreator
{
public:
	virtual ~BaseDataWidgetCreator() {}
	virtual BaseDataWidget* create(panda::BaseData* data, QWidget* parent) = 0;
};

class DataWidgetFactory
{
private:
	DataWidgetFactory() {}

public:
	typedef QSharedPointer<BaseDataWidgetCreator> DataWidgetCreatorPtr;

	class DataWidgetEntry
	{
	public:
		DataWidgetEntry() {}

		int fullType;
		QString widgetName;
		DataWidgetCreatorPtr creator;
	};

	static DataWidgetFactory* getInstance();
	DataWidgetEntry* getEntry(int fullType, QString widgetName);
	QList<QString> getWidgetNames(int fullType);

	BaseDataWidget* create(panda::BaseData* data, QWidget* parent);
protected:
	typedef QSharedPointer<DataWidgetEntry> DataWidgetEntryPtr;
	typedef QMap< int, QMap<QString, DataWidgetEntryPtr> > RegistryMap;
	RegistryMap registry;

	template<class T> friend class RegisterWidget;
	void registerWidget(int fullType, QString widgetName, DataWidgetCreatorPtr creator);
};

template<class T>
class DataWidgetCreator : public BaseDataWidgetCreator
{
public:
	virtual BaseDataWidget* create(panda::BaseData* data, QWidget* parent)
	{
		T::TData* tData = dynamic_cast<T::TData*>(data);
		if(!data)
			return nullptr;
		return new T(parent, tData);
	}
};

template <class T>
class RegisterWidget
{
public:
	explicit RegisterWidget(QString widgetName)
	{
		int fullType = panda::types::VirtualDataTrait<T::TData::value_type>::get()->fullTypeId();
		DataWidgetFactory::getInstance()->registerWidget(fullType, widgetName
							, QSharedPointer<DataWidgetCreator<T>>::create());
	}

private:
	RegisterWidget();
};

#endif // DATAWIDGETFACTORY_H
