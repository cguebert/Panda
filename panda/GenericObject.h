#ifndef GENERICOBJECT_H
#define GENERICOBJECT_H

#include <panda/PandaObject.h>
#include <panda/types/DataTypeId.h>

#include <panda/types/Color.h>
#include <panda/types/Gradient.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Path.h>
#include <panda/types/Topology.h>
#include <panda/types/Rect.h>

#include <QList>
#include <QMap>
#include <QSharedPointer>

#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>

class GenericObjectDrawStruct;

namespace panda
{

class GenericData : public Data<int>
{
public:
	explicit GenericData(const BaseData::BaseInitData& init)
		: Data<int>(init) {}

	virtual bool validParent(const BaseData* parent) const;
	virtual QString getDescription() const;
	virtual QString getTypesName() const;

	QList<int> allowedTypes;
};

class GenericSingleValueData : public GenericData
{
public:
	explicit GenericSingleValueData(const BaseData::BaseInitData& init)
		: GenericData(init) {}

	virtual bool validParent(const BaseData* parent) const;
	virtual QString getDescription() const;
};

class GenericVectorData : public GenericData
{
public:
	explicit GenericVectorData(const BaseData::BaseInitData& init)
		: GenericData(init) {}

	virtual bool validParent(const BaseData* parent) const;
	virtual QString getDescription() const;
};

class GenericAnimationData : public GenericData
{
public:
	explicit GenericAnimationData(const BaseData::BaseInitData& init)
		: GenericData(init) {}

	virtual bool validParent(const BaseData* parent) const;
	virtual QString getDescription() const;
};

class GenericSpecificData : public GenericData	// Accepts only the specific types given allowedTypes
{
public:
	explicit GenericSpecificData(const BaseData::BaseInitData& init)
		: GenericData(init) {}

	virtual bool validParent(const BaseData* parent) const;
	virtual QString getDescription() const;
};

//***************************************************************//

class GenericObject : public PandaObject
{
	friend class GenericObjectDrawStruct;
public:
	PANDA_CLASS(GenericObject, PandaObject)

	struct GenericDataDefinition
	{
		GenericDataDefinition()
			: type(0), input(false), output(false) {}
		GenericDataDefinition(int t, bool i, bool o, QString n, QString h)
			: type(t), input(i), output(o), name(n), help(h) {}

		int type;	// Leave the value type part at 0 to use the value type of the connected Data
		bool input, output;
		QString name, help;
	};
	typedef QList<GenericDataDefinition> GenericDataDefinitionList;
	typedef QList<BaseData*> DataList;

	explicit GenericObject(PandaDocument *parent = 0);
	virtual ~GenericObject();

	void setupGenericObject(GenericData* data, const GenericDataDefinitionList& defList);

	virtual void update();
	virtual void dataSetParent(BaseData* data, BaseData* parent);

	// use the GENERIC_OBJECT macro to create these functions
	virtual void invokeFunction(int type, DataList& list) = 0;
	virtual QList<int> getRegisteredTypes() = 0;

	virtual void save(QDomDocument& doc, QDomElement& elem, const QList<PandaObject*>* selected = nullptr);
	virtual void load(QDomElement& elem);

protected:
	virtual BaseData* createDatas(int type);
	virtual void updateDataNames();

private:
	virtual void registerFunctions() {}

	typedef QSharedPointer<BaseData> BaseDataPtr;
	typedef QList<BaseDataPtr> DataPtrList;

	struct CreatedDatasStruct
	{
		int type;
		DataPtrList datas;
	};

	GenericData* m_genericData;
	GenericDataDefinitionList m_dataDefinitions;

	typedef QSharedPointer<CreatedDatasStruct> CreatedDatasStructPtr;
	QList<CreatedDatasStructPtr> m_createdDatasStructs;
	QMap<BaseData*, CreatedDatasStructPtr> m_createdDatasMap;
};

//***************************************************************//

typedef boost::mpl::vector<int, PReal, types::Color, types::Point, types::Rect, QString, types::ImageWrapper, types::Gradient, types::Topology, types::Path> allDataTypes;
typedef boost::mpl::vector<int, PReal, types::Color, types::Point, types::Rect, QString> allSearchableTypes;
typedef boost::mpl::vector<int, PReal, types::Color, types::Point, types::Rect> allNumericalTypes;
typedef boost::mpl::vector<PReal, types::Color, types::Point, types::Gradient> allAnimationTypes;

#define GENERIC_OBJECT(T, L)								\
	protected:												\
	typedef void(T::*funcPtr)(DataList&);					\
	QMap<int, funcPtr> m_functions;							\
	struct functionCreatorWrapper							\
	{														\
		T* object;											\
		functionCreatorWrapper(T* obj) : object(obj) {}		\
		template<typename U> void operator()(U)				\
		{													\
			int type = types::DataTypeId::getIdOf<U>();		\
			object->registerFunction(type, &T::updateT<U>);	\
		}													\
	};														\
	void registerFunction(int type, funcPtr ptr)			\
	{														\
		m_functions[type] = ptr;							\
	}														\
	private:												\
	virtual void registerFunctions()						\
	{														\
		boost::mpl::for_each<L>								\
			(functionCreatorWrapper(this));					\
	}														\
	public:													\
	virtual QList<int> getRegisteredTypes()					\
	{														\
		return m_functions.keys();							\
	}														\
	virtual void invokeFunction(int type, DataList& list)	\
	{														\
		(this->*m_functions[type])(list);					\
	}

} // namespace panda

#endif // GENERICOBJECT_H
