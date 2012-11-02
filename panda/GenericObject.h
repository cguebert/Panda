#ifndef GENERICOBJECT_H
#define GENERICOBJECT_H

#include <panda/PandaObject.h>

#include <QList>
#include <QMap>
#include <QSharedPointer>

#include <boost/mpl/list.hpp>
#include <boost/mpl/for_each.hpp>

namespace panda
{

class GenericObject : public PandaObject
{
public:
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

	virtual void load(QDataStream& in);
	virtual void load(QTextStream& in);

protected:
	virtual BaseData* createDatas(int type);
	virtual void updateDataNames();

private:
	virtual void registerFunctions() {}

	struct CreatedDatasStruct
	{
		int type;
		DataList datas;
	};

	GenericData* genericData_;
	GenericDataDefinitionList dataDefinitions_;

//	typedef QSharedPointer<CreatedDatasStruct> CreatedDatasStructPtr;
	typedef CreatedDatasStruct* CreatedDatasStructPtr;
	QList<CreatedDatasStructPtr> createdDatasStructs_;
	QMap<BaseData*, CreatedDatasStruct*> createdDatasMap_;
	bool setParentProtection_;
};

typedef boost::mpl::list<int, double, QColor, QPointF, QRectF, QString, QImage> allDataTypes;
typedef boost::mpl::list<double, QColor, QPointF> allAnimationTypes;

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
