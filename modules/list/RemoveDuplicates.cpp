#include <panda/PandaDocument.h>
#include <panda/GenericObject.h>
#include <panda/ObjectFactory.h>
#include <set>

namespace std {

template<>
struct less<QColor> : public binary_function<QColor, QColor, bool>
{
	bool operator()(const QColor& lhs, const QColor& rhs) const
	{
		return lhs.rgb() < rhs.rgb();
	}
};

template<>
struct less<QPointF> : public binary_function<QPointF, QPointF, bool>
{
	bool operator()(const QPointF& lhs, const QPointF& rhs) const
	{
		return lhs.x() < rhs.x() || (lhs.x() == rhs.x() && lhs.y() < rhs.y());
	}
};

template<>
struct less<QRectF> : public binary_function<QRectF, QRectF, bool>
{
	bool operator()(const QRectF& lhs, const QRectF& rhs) const
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
};

}

namespace panda {

using types::DataTypeId;

class RemoveDuplicates : public GenericObject
{
	GENERIC_OBJECT(RemoveDuplicates, allSearchableTypes)
public:
	PANDA_CLASS(RemoveDuplicates, GenericObject)

	RemoveDuplicates(PandaDocument *doc)
		: GenericObject(doc)
		, generic(initData(&generic, "input", "Connect here the lists in which to remove duplicates"))
	{
		addInput(&generic);

		GenericDataDefinitionList defList;
		int listType = types::DataTypeId::getFullTypeOfVector(0);
		defList.append(GenericDataDefinition(listType,
											 true, false,
											 "input",
											 "Input list"));
		defList.append(GenericDataDefinition(listType,
											 false, true,
											 "output",
											 "Output list"));
		setupGenericObject(&generic, defList);
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< QVector<T> > ListData;
		ListData* dataInput = dynamic_cast<ListData*>(list[0]);
		ListData* dataOutput = dynamic_cast<ListData*>(list[1]);

		Q_ASSERT(dataInput && dataOutput);

		const QVector<T> &valIn = dataInput->getValue();
		auto valOut = dataOutput->getAccessor();

		std::set<T> tmpSet;

		valOut.clear();
		for(auto v : valIn)
		{
			if(tmpSet.find(v) == tmpSet.end())
			{
				tmpSet.insert(v);
				valOut.push_back(v);
			}
		}
	}

protected:
	GenericVectorData generic;
};

int RemoveDuplicatesClass = RegisterObject<RemoveDuplicates>("List/Remove duplicates").setDescription("Remove values that appear more than one time");

} // namespace Panda
