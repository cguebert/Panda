#ifndef GENERICOBJECT_H
#define GENERICOBJECT_H

#include <panda/PandaObject.h>

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

	QList<int> allowedTypes;
};

class GenericNonVectorData : public GenericData
{
public:
	explicit GenericNonVectorData(const BaseData::BaseInitData& init)
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

	virtual void save(QDataStream& out);
	virtual void save(QTextStream& out);
	virtual void save(QDomDocument& doc, QDomElement& elem);

	virtual void load(QDataStream& in);
	virtual void load(QTextStream& in);
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

	GenericData* genericData_;
	GenericDataDefinitionList dataDefinitions_;

	typedef QSharedPointer<CreatedDatasStruct> CreatedDatasStructPtr;
	QList<CreatedDatasStructPtr> createdDatasStructs_;
	QMap<BaseData*, CreatedDatasStructPtr> createdDatasMap_;
};

//***************************************************************//

typedef boost::mpl::vector<int, double, QColor, QPointF, QRectF, QString, QImage> allDataTypes;
typedef boost::mpl::vector<int, double, QColor, QPointF, QRectF> allNumericalTypes;
typedef boost::mpl::vector<double, QColor, QPointF> allAnimationTypes;

#define GENERIC_OBJECT(T, L)								\
	protected:												\
	typedef void(T::*funcPtr)(DataList&);					\
	QMap<int, funcPtr> functions;							\
	struct functionCreatorWrapper							\
	{														\
		T* object;											\
		functionCreatorWrapper(T* obj) : object(obj) {}		\
		template<typename U> void operator()(U)				\
		{													\
			int type = BaseData::getValueTypeOf<U>();		\
			object->registerFunction(type, &T::updateT<U>);	\
		}													\
	};														\
	void registerFunction(int type, funcPtr ptr)			\
	{														\
		functions[type] = ptr;								\
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
		return functions.keys();							\
	}														\
	virtual void invokeFunction(int type, DataList& list)	\
	{														\
		(this->*functions[type])(list);						\
	}

} // namespace panda

#endif // GENERICOBJECT_H
