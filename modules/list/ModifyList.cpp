#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>

#include <QMetaType>

namespace panda {

template <class T>
class ConcatItems : public PandaObject
{
public:
	ConcatItems(PandaDocument *doc)
        : PandaObject(doc)
		, inputA(initData(&inputA, "head", "List of items that will be at the head of the result"))
		, inputB(initData(&inputB, "tail", "List of items that will be at the tail of the result"))
		, result(initData(&result, "result", "Result of the concatenation"))
    {
		addInput(&inputA);
		addInput(&inputB);

		addOutput(&result);
    }

    void update()
    {
		QVector<T>	&valInA = *inputA.beginEdit(),
					&valInB = *inputB.beginEdit(),
					&valOut = *result.beginEdit();

		valOut.clear();
		valOut << valInA << valInB;

        this->cleanDirty();
    }

protected:
	Data< QVector<T> > inputA, inputB, result;
};

int ConcatItemsIntClass = RegisterObject("List/Integer/Concatenation").setClass< ConcatItems<int> >().setDescription("Concatenate 2 lists of integers");
int ConcatItemsDoubleClass = RegisterObject("List/Real/Concatenation").setClass< ConcatItems<double> >().setDescription("Concatenate 2 lists of doubles");
int ConcatItemsColorClass = RegisterObject("List/Color/Concatenation").setClass< ConcatItems<QColor> >().setDescription("Concatenate 2 lists of colors");
int ConcatItemsPointClass = RegisterObject("List/Point/Concatenation").setClass< ConcatItems<QPointF> >().setDescription("Concatenate 2 lists of points");
int ConcatItemsRectClass = RegisterObject("List/Rectangle/Concatenation").setClass< ConcatItems<QRectF> >().setDescription("Concatenate 2 lists of rectangles");
int ConcatItemsStringClass = RegisterObject("List/Text/Concatenation").setClass< ConcatItems<QString> >().setDescription("Concatenate 2 lists of texts");

//*************************************************************************//

template <class T>
class ExtractHead : public PandaObject
{
public:
	ExtractHead(PandaDocument *doc)
		: PandaObject(doc)
		, value(initData(&value, 1, "head", "This number of items will be extracted from the start of the list"))
		, input(initData(&input, "input", "List of items from which to extract the head"))
		, result(initData(&result, "result", "Result of the extraction"))
	{
		addInput(&value);
		addInput(&input);

		addOutput(&result);
	}

	void update()
	{
		int val = value.getValue();
		QVector<T>	&valIn = *input.beginEdit(),
					&valOut = *result.beginEdit();

		valOut.swap( valIn.mid(0, val) );

		this->cleanDirty();
	}

protected:
	Data< int > value;
	Data< QVector<T> > input, result;
};

int ExtractHeadIntClass = RegisterObject("List/Integer/Get head").setClass< ExtractHead<int> >().setDescription("Get the first items of a list of integers");
int ExtractHeadDoubleClass = RegisterObject("List/Real/Get head").setClass< ExtractHead<double> >().setDescription("Get the first items of a list of doubles");
int ExtractHeadColorClass = RegisterObject("List/Color/Get head").setClass< ExtractHead<QColor> >().setDescription("Get the first items of a list of colors");
int ExtractHeadPointClass = RegisterObject("List/Point/Get head").setClass< ExtractHead<QPointF> >().setDescription("Get the first items of a list of points");
int ExtractHeadRectClass = RegisterObject("List/Rectangle/Get head").setClass< ExtractHead<QRectF> >().setDescription("Get the first items of a list of rectangles");
int ExtractHeadStringClass = RegisterObject("List/Text/Get head").setClass< ExtractHead<QString> >().setDescription("Get the first items of a list of texts");

//*************************************************************************//

template <class T>
class ExtractTail : public PandaObject
{
public:
	ExtractTail(PandaDocument *doc)
		: PandaObject(doc)
		, value(initData(&value, 1, "tail", "All items starting at this index will be extracted. If negative, extract this number of items instead."))
		, input(initData(&input, "input", "List of items from which to extract the tail"))
		, result(initData(&result, "result", "Result of the extraction"))
	{
		addInput(&value);
		addInput(&input);

		addOutput(&result);
	}

	void update()
	{
		int val = value.getValue();
		QVector<T>	&valIn = *input.beginEdit(),
					&valOut = *result.beginEdit();

		if(val > 0)
			valOut.swap( valIn.mid(val) );
		else
			valOut.swap( valIn.mid(valIn.size() + val) );

		this->cleanDirty();
	}

protected:
	Data< int > value;
	Data< QVector<T> > input, result;
};

int ExtractTailIntClass = RegisterObject("List/Integer/Get tail").setClass< ExtractTail<int> >().setDescription("Get the last items of a list of integers");
int ExtractTailDoubleClass = RegisterObject("List/Real/Get tail").setClass< ExtractTail<double> >().setDescription("Get the last items of a list of doubles");
int ExtractTailColorClass = RegisterObject("List/Color/Get tail").setClass< ExtractTail<QColor> >().setDescription("Get the last items of a list of colors");
int ExtractTailPointClass = RegisterObject("List/Point/Get tail").setClass< ExtractTail<QPointF> >().setDescription("Get the last items of a list of points");
int ExtractTailRectClass = RegisterObject("List/Rectangle/Get tail").setClass< ExtractTail<QRectF> >().setDescription("Get the last items of a list of rectangles");
int ExtractTailStringClass = RegisterObject("List/Text/Get tail").setClass< ExtractTail<QString> >().setDescription("Get the last items of a list of texts");

//*************************************************************************//

template <class T>
class DeleteItems : public PandaObject
{
public:
	DeleteItems(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "items", "The initial list of items"))
		, indices(initData(&indices, "indices", "Indices of items to be removed from the list"))
		, result(initData(&result, "result", "Result of the deletion"))
	{
		addInput(&indices);
		addInput(&input);

		addOutput(&result);
	}

	void update()
	{
		const QVector<T> &valIn = input.getValue();
		QVector<T> &valOut = *result.beginEdit();
		const QVector<int> &valId = indices.getValue();

		QVector<bool> mask;
		unsigned int nbIn = valIn.size(), nbId = valId.size();
		mask.fill(true, nbIn);
		for(unsigned int i=0; i<nbId; ++i)
		{
			unsigned int id = valId[i];
			if(id < nbIn)
				mask[id] = false;	// Mark for deletion
		}

		valOut.clear();
		for(unsigned int i=0; i<nbIn; ++i)
		{
			if(mask[i])
				valOut << valIn[i];
		}

		this->cleanDirty();
	}

protected:
	Data< QVector<T> > input, result;
	Data< QVector<int> > indices;
};

int DeleteItemsIntClass = RegisterObject("List/Integer/Delete items").setClass< DeleteItems<int> >().setDescription("Remove some items from a list of integers");
int DeletetItemsDoubleClass = RegisterObject("List/Real/Delete items").setClass< DeleteItems<double> >().setDescription("Remove some items from a list of doubles");
int DeleteItemsColorClass = RegisterObject("List/Color/Delete items").setClass< DeleteItems<QColor> >().setDescription("Remove some items from a list of colors");
int DeleteItemsPointClass = RegisterObject("List/Point/Delete items").setClass< DeleteItems<QPointF> >().setDescription("Remove some items from a list of points");
int DeleteItemsRectClass = RegisterObject("List/Rectangle/Delete items").setClass< DeleteItems<QRectF> >().setDescription("Remove some items from a list of rectangles");
int DeleteItemsStringClass = RegisterObject("List/Text/Delete items").setClass< DeleteItems<QString> >().setDescription("Remove some items from a list of texts");


} // namespace Panda
