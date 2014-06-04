#ifndef GENERICOBJECT_H
#define GENERICOBJECT_H

#include <panda/PandaObject.h>
#include <panda/types/DataTypeId.h>

#include <panda/types/Color.h>
#include <panda/types/Gradient.h>
#include <panda/types/ImageWrapper.h>
#include <panda/types/Path.h>
#include <panda/types/Mesh.h>
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
	virtual QString getTypesName(bool useFullDescription = false) const;
	virtual int getCompatibleType(const BaseData* parent) const;

	QList<int> m_allowedTypes;
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
	virtual int getCompatibleType(const BaseData* parent) const;
};

//****************************************************************************//

class GenericObject : public PandaObject
{
public:
	PANDA_CLASS(GenericObject, PandaObject)

	struct GenericDataDefinition
	{
		GenericDataDefinition()
			: type(0), input(false), output(false) {}
		GenericDataDefinition(int type, bool input, bool output, QString name, QString help)
			: type(type), input(input), output(output), name(name), help(help) {}

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

	virtual BaseData* createDatas(int type, int index = -1);

protected:
	virtual void reorderDatas();
	virtual void updateDataNames();

	GenericData* const getGenericData(); // Access to m_genericData
	int nbOfCreatedDatas() const; // Size of m_createdDatasStructs
	bool isCreatedData(BaseData* data) const; // Return true if data has been created by the GenericObject

private:
	friend class SingleTypeGenericObject;
	friend class GenericObjectDrawStruct;

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

	void createUndoCommands(const CreatedDatasStructPtr& createdData);
};

//****************************************************************************//

// Works as a GenericObject, but as soon as one input Data is connected,
// it only accepts this type for all future connections.
class SingleTypeGenericObject : public GenericObject
{
public:
	PANDA_CLASS(SingleTypeGenericObject, GenericObject)

	explicit SingleTypeGenericObject(PandaDocument *parent = 0);

	virtual void update();
	virtual BaseData* createDatas(int type, int index = -1);
	virtual void dataSetParent(BaseData* data, BaseData* parent);

protected:
	bool m_singleOutput; // Set this to true to only create outputs for the first connected Data

private:
	int m_connectedType;
};

//****************************************************************************//

typedef boost::mpl::vector<int, PReal, types::Color, types::Point, types::Rect, QString, types::ImageWrapper, types::Gradient, types::Mesh, types::Path> allDataTypes;
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
