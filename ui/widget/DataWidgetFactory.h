#ifndef DATAWIDGETFACTORY_H
#define DATAWIDGETFACTORY_H

#include <panda/types/DataTraits.h>

#include <map>
#include <memory>

#include <QString>

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
	virtual BaseDataWidget* create(QWidget* parent, panda::BaseData* data) const = 0;
	virtual BaseDataWidget* create(QWidget* parent, void* pValue,
								   QString widgetName, QString name, QString parameters) const = 0;
	virtual QString getParametersFormat() = 0;
};

class DataWidgetFactory
{
private:
	DataWidgetFactory() {}

public:
	typedef std::shared_ptr<BaseDataWidgetCreator> DataWidgetCreatorPtr;

	class DataWidgetEntry
	{
	public:
		DataWidgetEntry() {}

		int fullType;
		QString widgetName;
		DataWidgetCreatorPtr creator;
	};

	static DataWidgetFactory* getInstance();
	const DataWidgetEntry* getEntry(int fullType, QString widgetName) const;
	const BaseDataWidgetCreator* getCreator(int fullType, QString widgetName) const;
	std::vector<QString> getWidgetNames(int fullType) const;

	BaseDataWidget* create(QWidget* parent, panda::BaseData* data) const;
	BaseDataWidget* create(QWidget* parent, void* pValue, int fullType,
						   QString widget, QString displayName, QString parameters) const;

protected:
	typedef std::shared_ptr<DataWidgetEntry> DataWidgetEntryPtr;
	typedef std::map< int, std::map<QString, DataWidgetEntryPtr> > RegistryMap;
	RegistryMap registry;

	template<class T> friend class RegisterWidget;
	void registerWidget(int fullType, QString widgetName, DataWidgetCreatorPtr creator);
};

template<class T>
class DataWidgetCreator : public BaseDataWidgetCreator
{
public:
	virtual BaseDataWidget* create(QWidget* parent, panda::BaseData* data) const
	{
		T::TData* tData = dynamic_cast<T::TData*>(data);
		if(!data)
			return nullptr;
		return new T(parent, tData);
	}

	virtual BaseDataWidget* create(QWidget* parent, void* pValue,
								   QString widgetName, QString name, QString parameters) const
	{
		T::TData::value_type* tValue = reinterpret_cast<T::TData::value_type*>(pValue);
		if(!tValue)
			return nullptr;
		return new T(parent, tValue, widgetName, name, parameters);
	}

	virtual QString getParametersFormat()
	{
		return ParametersFormatHelper<T>();
	}
};

template <class T>
class RegisterWidget
{
public:
	explicit RegisterWidget(QString widgetName)
	{
		// Ensure the value type is already registered
		typedef panda::types::DataTrait<T::TData::value_type> Trait;
		panda::types::DataTypeId::registerType<Trait::value_type>(panda::types::DataTrait<Trait::value_type>::fullTypeId());
		panda::types::DataTypeId::registerType<T::TData::value_type>(panda::types::DataTrait<T::TData::value_type>::fullTypeId());

		int fullType = panda::types::VirtualDataTrait<T::TData::value_type>::get()->fullTypeId();
		DataWidgetFactory::getInstance()->registerWidget(fullType, widgetName
							, std::make_shared<DataWidgetCreator<T>>());
	}

private:
	RegisterWidget();
};

#endif // DATAWIDGETFACTORY_H
