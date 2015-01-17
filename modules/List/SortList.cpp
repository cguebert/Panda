#include <panda/PandaDocument.h>
#include <panda/ObjectFactory.h>
#include <panda/GenericObject.h>

#include <QMap>

using panda::types::Color;
using panda::types::Point;
using panda::types::Rect;

template<>
static bool qMapLessThanKey<Color>(const Color& lhs, const Color& rhs)
{
	return lhs.toHex() < rhs.toHex();
}

template<>
static bool qMapLessThanKey<Point>(const Point& p1, const Point& p2)
{
	return p1.x < p2.x || (p1.x == p2.x && p1.y < p2.y);
}

template<>
static bool qMapLessThanKey<Rect>(const Rect& lhs, const Rect& rhs)
{
	if(lhs.left() < rhs.left()) return true;
	if(rhs.left() < lhs.left()) return false;

	if(lhs.top() < rhs.top()) return true;
	if(rhs.top() < lhs.top()) return false;

	if(lhs.right() < rhs.right()) return true;
	if(rhs.right() < lhs.right()) return false;

	if(lhs.bottom() < rhs.bottom()) return true;
	if(rhs.bottom() < lhs.bottom()) return false;

	return false;
}

namespace panda {

using types::DataTypeId;

class SortList  : public GenericObject
{
	GENERIC_OBJECT(SortList, allSearchableTypes)
public:
	PANDA_CLASS(SortList, GenericObject)

	SortList(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData(&generic, "input", "Connect here the lists to get the items from"))
	{
		addInput(generic);

		GenericDataDefinitionList defList;
		int listType = DataTypeId::getFullTypeOfVector(0);
		defList.push_back(GenericDataDefinition(listType,
											 true, false,
											 "input",
											 "List to sort"));
		defList.push_back(GenericDataDefinition(listType,
											 false, true,
											 "output",
											 "Sorted list"));
		defList.push_back(GenericDataDefinition(DataTypeId::getFullTypeOfVector(DataTypeId::getIdOf<int>()),
											 false, true,
											 "indices",
											 "Indices corresponding to the sorted list"));
		setupGenericObject(generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< QVector<T> > VecData;
		typedef Data< QVector<int> > VecIntData;
		VecData* dataInput = dynamic_cast<VecData*>(list[0]);
		VecData* dataOutput = dynamic_cast<VecData*>(list[1]);
		VecIntData* dataIndices = dynamic_cast<VecIntData*>(list[2]);

		Q_ASSERT(dataInput && dataOutput && dataIndices);

		const QVector<T> &valIn = dataInput->getValue();

		QMap<T, int> tmpMap;
		int nb = valIn.size();

		for(int i=0; i<nb; ++i)
			tmpMap.insertMulti(valIn[i], i);

		dataOutput->setValue(tmpMap.keys().toVector());
		dataIndices->setValue(tmpMap.values().toVector());
	}

protected:
	GenericVectorData generic;
};

int SortListClass = RegisterObject<SortList>("List/Sort list").setDescription("Sort a list");

} // namespace Panda
