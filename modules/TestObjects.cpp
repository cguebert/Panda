#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <QRectF>
#include <QVector>

#include <math.h>

namespace panda {

class TestObjectA : public PandaObject
{
public:
	TestObjectA(PandaDocument *doc)
		: PandaObject(doc)
		, testInputInt(initData(&testInputInt, 5, "inputIntData", "Test of a integer Data"))
		, testOutputInt(initData(&testOutputInt, 0, "outputIntData", "Test of a integer Data " ))
		, testInputDouble(initData(&testInputDouble, 2.5, "inputDoubleData", "Test of a double Data"))
		, testOutputDouble(initData(&testOutputDouble, 0.0, "outputDoubleData", "Test of a double Data"))
		, testOuputRect(initData(&testOuputRect, "ouputRectData", "A QRectF Data"))
		, testColor(initData(&testColor, "color", "Test of a non-connected color Data"))
	{
		addInput(&testInputInt);
		addInput(&testInputDouble);
		addInput(&testColor);

		addOutput(&testOutputInt);
		addOutput(&testOutputDouble);
		addOutput(&testOuputRect);

		testColor.setValue(QColor(128, 255, 0, 96));
	}

	void update()
	{
		int inInt = testInputInt.getValue();
		double inDouble = testInputDouble.getValue();
		double result = inInt * inDouble;

		testOutputDouble.setValue(result - floor(result));
		testOutputInt.setValue(floor(result));

		testOuputRect.setValue(QRectF(-result, -result, inInt, inInt));

		this->cleanDirty();
	}

protected:
	Data<int> testInputInt, testOutputInt;
	Data<double> testInputDouble, testOutputDouble;
	Data<QRectF> testOuputRect;
	Data<QColor> testColor;
};

class TestObjectB : public PandaObject
{
public:
	TestObjectB(PandaDocument *doc)
		: PandaObject(doc)
		, testOutputInt(initData(&testOutputInt, 0, "outputIntData", "Test of a integer Data" ))
		, testOutputDouble(initData(&testOutputDouble, 0.0, "outputDoubleData", "Test of a double Data"))
		, testInputRect(initData(&testInputRect, "inputRectData", "A QRectF Data"))
		, testInputPoint(initData(&testInputPoint, "inputPointData", "A QPointF Data"))
	{
		addInput(&testInputRect);
		addInput(&testInputPoint);

		addOutput(&testOutputInt);
		addOutput(&testOutputDouble);
	}

	void update()
	{
		QPointF pt = testInputPoint.getValue();
		testOutputInt.setValue(pt.x()+pt.y());

		QRectF rect = testInputRect.getValue();
		double w = rect.width(), h = rect.height();
		testOutputDouble.setValue(sqrt(w*w+h*h));

		this->cleanDirty();
	}

protected:
	Data<int> testOutputInt;
	Data<double> testOutputDouble;
	Data<QRectF> testInputRect;
	Data<QPointF> testInputPoint;
};

class TestObjectC : public PandaObject
{
public:
	TestObjectC(PandaDocument *doc)
		: PandaObject(doc)
		, vectorInt(initData(&vectorInt, "vectorInt", "Vector of integers" ))
		, vectorPoints(initData(&vectorPoints, "vectorPoints", "Vector of points"))
		, vectorRects(initData(&vectorRects, "vectorRects", "Vector of rectangles"))
		, vectorColors(initData(&vectorColors, "vectorColors", "Vector of colors"))
	{
		addInput(&vectorInt);
		addInput(&vectorPoints);

		addOutput(&vectorRects);
		addOutput(&vectorColors);

		QVector<int>& vecInt = *vectorInt.beginEdit();
		vecInt << 2 << 4 << 6 << 8;
		vectorInt.endEdit();

		QVector<QPointF>& vecPts = *vectorPoints.beginEdit();
		vecPts << QPointF(50, 100) << QPointF(200, 200) << QPointF(150, 50);
		vectorPoints.endEdit();

		QVector<QColor>& vecColors = *vectorColors.beginEdit();
		vecColors << QColor(255,0,0) << QColor(0,255,0) << QColor(0,0,255);
		vectorColors.endEdit();
	}

	void update()
	{
		const QVector<int>& vecInt = vectorInt.getValue();
		const QVector<QPointF>& vecPts = vectorPoints.getValue();

		int count = qMin(vecInt.count(), vecPts.count());
		QVector<QRectF>& vecRects = *vectorRects.beginEdit();
		vecRects.clear();
		for(int i=0; i<count; ++i)
			vecRects.append(QRectF(vecPts[i], QSizeF(vecInt[i], vecInt[i])));
		vectorRects.endEdit();

		this->cleanDirty();
	}

protected:
	Data< QVector<int> > vectorInt;
	Data< QVector<QPointF> > vectorPoints;
	Data< QVector<QRectF> > vectorRects;
	Data< QVector<QColor> > vectorColors;
};

class TestObjectD : public PandaObject
{
public:
	TestObjectD(PandaDocument *doc)
		: PandaObject(doc)
		, intInput(initData(&intInput, "intInput", "Integer" ))
		, vectorDouble(initData(&vectorDouble, "vectorDouble", "Vector of double" ))
		, vectorPoints(initData(&vectorPoints, "vectorPoints", "Vector of points"))
		, vectorRects(initData(&vectorRects, "vectorRects", "Vector of rectangles"))
		, vectorColors(initData(&vectorColors, "vectorColors", "Vector of colors"))
		, vectorStrings(initData(&vectorStrings, "vectorStrings", "Vector of strings"))
	{
		addInput(&intInput);
		addInput(&vectorDouble);
		addInput(&vectorRects);
		addInput(&vectorColors);

		addOutput(&vectorPoints);
		addOutput(&vectorStrings);
	}

	void update()
	{

		const QVector<QRectF>& vecRects = vectorRects.getValue();

		QVector<QPointF>* vecPts = vectorPoints.beginEdit();
		vecPts->clear();
		for(int i=0; i<vecRects.size(); ++i)
		{
			vecPts->append(vecRects[i].topLeft());
			vecPts->append(vecRects[i].bottomRight());
		}
		vectorPoints.endEdit();

		const int intVal = intInput.getValue();
		const QVector<double>& vecDouble = vectorDouble.getValue();
		QVector<QString>* vecStrings = vectorStrings.beginEdit();
		vecStrings->clear();
		for(int i=0; i<vecDouble.size(); ++i)
			vecStrings->append(QString::number(vecDouble[i]*intVal));
		vectorStrings.endEdit();

		this->cleanDirty();
	}

protected:
	Data< int > intInput;
	Data< QVector<double> > vectorDouble;
	Data< QVector<QPointF> > vectorPoints;
	Data< QVector<QRectF> > vectorRects;
	Data< QVector<QColor> > vectorColors;
	Data< QVector<QString> > vectorStrings;
};

int TestObjectAClass = RegisterObject("Tests/Test A").setClass<TestObjectA>().setDescription("This is a simple object for test purposes");
int TestObjectBClass = RegisterObject("Tests/Test B").setClass<TestObjectB>().setDescription("This is another object for the tests");
int TestObjectCClass = RegisterObject("Tests/Test C").setClass<TestObjectC>().setDescription("Yet another object for tests");
int TestObjectDClass = RegisterObject("Tests/Test D").setClass<TestObjectD>().setDescription("Maybe the last one ?");

} // namespace Panda


