#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <modules/generators/UserValue.h>

#include <panda/types/Animation.h>
#include <panda/types/Gradient.h>
#include <panda/types/Path.h>
#include <panda/types/Rect.h>
#include <QColor>
#include <QString>
#include <QVector>

#include <QTimer>
#include <QFile>
#include <QDomDocument>

namespace panda {

template <class T>
class CustomData : public Data<T>
{
public:
	CustomData(const QString& name, const QString& help, PandaObject* owner)
		: Data<T>(name, help, owner)
	{ }
	virtual void save(QDomDocument& doc, QDomElement& elem)
	{
		Data<T>::save(doc, elem);
		QString w = getWidget();
		if(w != "default")
			elem.setAttribute("widget", w);
		QString d = getWidgetData();
		if(!d.isEmpty())
			elem.setAttribute("widgetData", d);
	}
};

template <class T>
class GeneratorUser : public PandaObject
{
public:
	PANDA_CLASS(PANDA_TEMPLATE(GeneratorUser, T), PandaObject)

	GeneratorUser(PandaDocument *doc)
		: PandaObject(doc)
		, userValue("input", "The value you want to store", this)
		, output(initData(&output, "value", "The value stored"))
	{
		addInput(&userValue);
		addOutput(&output);

		output.setDisplayed(false);

		dataSetParent(&output, &userValue);
	}

protected:
	CustomData<T> userValue;
	Data<T> output;
};

int GeneratorUser_IntegerClass = RegisterObject< GeneratorUser<int> >("Generator/Integer/Integer user value").setName("Integer value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_DoubleClass = RegisterObject< GeneratorUser<PReal> >("Generator/Real/Real user value").setName("Real value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_PointClass = RegisterObject< GeneratorUser<types::Point> >("Generator/Point/Point user value").setName("Point value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_ColorClass = RegisterObject< GeneratorUser<QColor> >("Generator/Color/Color user value").setName("Color value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_RectClass = RegisterObject< GeneratorUser<types::Rect> >("Generator/Rectangle/Rectangle user value").setName("Rectangle value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_StringClass = RegisterObject< GeneratorUser<QString> >("Generator/Text/Text user value").setName("Text value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_GradientClass = RegisterObject< GeneratorUser<types::Gradient> >("Generator/Gradient/Gradient user value").setName("Gradient value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_PathClass = RegisterObject< GeneratorUser<types::Path> >("Generator/Path/Path user value").setName("Path value").setDescription("Lets you store a value for use in other objects");

int GeneratorUser_VectorIntegerClass = RegisterObject< GeneratorUser< QVector<int> > >("Generator/Integer/Integers list user value").setName("Integers list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorDoubleClass = RegisterObject< GeneratorUser< QVector<PReal> > >("Generator/Real/Reals list user value").setName("Reals list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorPointClass = RegisterObject< GeneratorUser< QVector<types::Point> > >("Generator/Point/Points list user value").setName("Points list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorColorClass = RegisterObject< GeneratorUser< QVector<QColor> > >("Generator/Color/Colors list user value").setName("Colors list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorRectClass = RegisterObject< GeneratorUser< QVector<types::Rect> > >("Generator/Rectangle/Rectangles list user value").setName("Rectangles list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorStringClass = RegisterObject< GeneratorUser< QVector<QString> > >("Generator/Text/Texts list user value").setName("Texts list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorGradientClass = RegisterObject< GeneratorUser< QVector<types::Gradient> > >("Generator/Gradient/Gradients list user value").setName("Gradients list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorPathClass = RegisterObject< GeneratorUser< QVector<types::Path> > >("Generator/Path/Paths list user value").setName("Paths list").setDescription("Lets you store a value for use in other objects");

int GeneratorUser_AnimationDoubleClass = RegisterObject< GeneratorUser< types::Animation<PReal> > >("Generator/Real/Reals animation user value").setName("Reals animation").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_AnimationPointClass = RegisterObject< GeneratorUser< types::Animation<types::Point> > >("Generator/Point/Points animation user value").setName("Points animation").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_AnimationColorClass = RegisterObject< GeneratorUser< types::Animation<QColor> > >("Generator/Color/Colors animation user value").setName("Colors animation").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_AnimationGradientClass = RegisterObject< GeneratorUser< types::Animation<types::Gradient> > >("Generator/Gradient/Gradients animation user value").setName("Gradients animation").setDescription("Lets you store a value for use in other objects");

//*************************************************************************//

template <class T>
class StoreValue : public PandaObject, public TimedMethodObject
{
public:
	PANDA_CLASS(PANDA_TEMPLATE(StoreValue, T), PandaObject)

	StoreValue(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData(&input, "input", "The value you want to store"))
		, fileName(initData(&fileName, "file name", "File where to store the value"))
		, singleValue(initData(&singleValue, 1, "single value", "If false save all the values during the animation"))
		, saveTimer(nullptr)
	{
		addInput(&input);
		addInput(&fileName);
		addInput(&singleValue);

		fileName.setWidget("save file");
		singleValue.setWidget("checkbox");

		TimedMethodObject* tmo = dynamic_cast<TimedMethodObject*>(this);

		saveTimer = new QTimer(tmo);
		saveTimer->setSingleShot(true);

		QObject::connect(saveTimer, SIGNAL(timeout()), tmo, SLOT(onTimeout()));
	}

	void removeChilds(QDomNode& node)
	{
		auto children = node.childNodes();
		int nb=children.size();
		for(int i=nb-1; i>=0; --i)
			node.removeChild(children.at(i));
	}

	void reset()
	{
		if(!xmlRoot.isNull())
			removeChilds(xmlRoot);
	}

	void addValue()
	{
		if(xmlRoot.isNull())
		{
			xmlRoot = xmlDoc.createElement("PandaValue");
			xmlDoc.appendChild(xmlRoot);
		}

		if(singleValue.getValue() || !isInStep)
			removeChilds(xmlRoot);

		QDomElement xmlData = xmlDoc.createElement("SavedData");
		input.save(xmlDoc, xmlData);
		xmlRoot.appendChild(xmlData);
	}

/*	void setDirtyValue()
	{
		PandaObject::setDirtyValue();
		if(!isInStep && !singleValue.getValue())
		{
			addValue();
			saveToFile();
		}
	}
*/
	void endStep()
	{
		addValue();
		PandaObject::endStep();

		saveTimer->stop();
		saveTimer->start(500);
	}

	void saveToFile()
	{
		QString tmpFileName = fileName.getValue();
		if(tmpFileName.isEmpty())
			return;
		QFile file(tmpFileName);
		if (!file.open(QIODevice::WriteOnly))
			return;

		file.write(xmlDoc.toByteArray(4));
	}

	void onTimeout()
	{
		saveToFile();
	}

protected:
	Data<T> input;
	Data<QString> fileName;
	Data<int> singleValue;
	QDomDocument xmlDoc;
	QDomElement xmlRoot;
	QTimer* saveTimer;
};

int StoreValue_IntegerClass = RegisterObject< StoreValue<int> >("File/Save integer").setDescription("Save a value in a file for later use");

//*************************************************************************//

template <class T>
class LoadValue : public PandaObject
{
public:
	PANDA_CLASS(PANDA_TEMPLATE(LoadValue, T), PandaObject)

	LoadValue(PandaDocument *doc)
		: PandaObject(doc)
		, output(initData(&output, "output", "The loaded value"))
		, fileName(initData(&fileName, "file name", "File where to read the value"))
	{
		addInput(&fileName);
		fileName.setWidget("open file");

		addOutput(&output);
	}

	void reset()
	{
		QString tmpFileName = fileName.getValue();
		if(tmpFileName.isEmpty())
			return;
		QFile file(tmpFileName);
		if (!file.open(QIODevice::ReadOnly))
			return;

		QDomDocument doc;
		int errLine, errCol;
		if (!doc.setContent(&file, nullptr, &errLine, &errCol))
			return;

		QDomElement root = doc.documentElement();
		xmlData = root.firstChildElement("SavedData");
	}

	void beginStep()
	{
		PandaObject::beginStep();

		if(!xmlData.isNull())
		{
			output.load(xmlData);

			xmlData = xmlData.nextSiblingElement("SavedData");
		}
	}

protected:
	Data<T> output;
	Data<QString> fileName;
	QDomElement xmlData;
};

int LoadValue_IntegerClass = RegisterObject< LoadValue<int> >("File/Load integer").setDescription("Load a value from a file");

} // namespace Panda
