#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <modules/generators/UserValue.h>

#include <panda/types/Animation.h>
#include <panda/types/Color.h>
#include <panda/types/Gradient.h>
#include <panda/types/Mesh.h>
#include <panda/types/Path.h>
#include <panda/types/Rect.h>
#include <panda/types/Shader.h>

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
class GeneratorUser : public BaseGeneratorUser
{
public:
	PANDA_CLASS(PANDA_TEMPLATE(GeneratorUser, T), BaseGeneratorUser)

	GeneratorUser(PandaDocument *doc)
		: BaseGeneratorUser(doc)
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

int GeneratorUser_ColorClass = RegisterObject< GeneratorUser<types::Color> >("Generator/Color/Color user value").setName("Color value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_DoubleClass = RegisterObject< GeneratorUser<PReal> >("Generator/Real/Real user value").setName("Real value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_GradientClass = RegisterObject< GeneratorUser<types::Gradient> >("Generator/Gradient/Gradient user value").setName("Gradient value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_IntegerClass = RegisterObject< GeneratorUser<int> >("Generator/Integer/Integer user value").setName("Integer value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_MeshClass = RegisterObject< GeneratorUser<types::Mesh> >("Generator/Mesh/Mesh user value").setName("Mesh value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_PathClass = RegisterObject< GeneratorUser<types::Path> >("Generator/Path/Path user value").setName("Path value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_PointClass = RegisterObject< GeneratorUser<types::Point> >("Generator/Point/Point user value").setName("Point value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_RectClass = RegisterObject< GeneratorUser<types::Rect> >("Generator/Rectangle/Rectangle user value").setName("Rectangle value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_ShaderClass = RegisterObject< GeneratorUser<types::Shader> >("Generator/Shader/Shader user value").setName("Shader value").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_StringClass = RegisterObject< GeneratorUser<QString> >("Generator/Text/Text user value").setName("Text value").setDescription("Lets you store a value for use in other objects");

int GeneratorUser_VectorColorClass = RegisterObject< GeneratorUser< QVector<types::Color> > >("Generator/Color/Colors list user value").setName("Colors list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorDoubleClass = RegisterObject< GeneratorUser< QVector<PReal> > >("Generator/Real/Reals list user value").setName("Reals list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorGradientClass = RegisterObject< GeneratorUser< QVector<types::Gradient> > >("Generator/Gradient/Gradients list user value").setName("Gradients list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorIntegerClass = RegisterObject< GeneratorUser< QVector<int> > >("Generator/Integer/Integers list user value").setName("Integers list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorMeshClass = RegisterObject< GeneratorUser< QVector<types::Mesh> > >("Generator/Mesh/Meshes list user value").setName("Meshes list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorPathClass = RegisterObject< GeneratorUser< QVector<types::Path> > >("Generator/Path/Paths list user value").setName("Paths list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorPointClass = RegisterObject< GeneratorUser< QVector<types::Point> > >("Generator/Point/Points list user value").setName("Points list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorRectClass = RegisterObject< GeneratorUser< QVector<types::Rect> > >("Generator/Rectangle/Rectangles list user value").setName("Rectangles list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorShaderClass = RegisterObject< GeneratorUser< QVector<types::Shader> > >("Generator/Shader/Shaders list user value").setName("Shaders list").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_VectorStringClass = RegisterObject< GeneratorUser< QVector<QString> > >("Generator/Text/Texts list user value").setName("Texts list").setDescription("Lets you store a value for use in other objects");

int GeneratorUser_AnimationColorClass = RegisterObject< GeneratorUser< types::Animation<types::Color> > >("Generator/Color/Colors animation user value").setName("Colors animation").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_AnimationDoubleClass = RegisterObject< GeneratorUser< types::Animation<PReal> > >("Generator/Real/Reals animation user value").setName("Reals animation").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_AnimationGradientClass = RegisterObject< GeneratorUser< types::Animation<types::Gradient> > >("Generator/Gradient/Gradients animation user value").setName("Gradients animation").setDescription("Lets you store a value for use in other objects");
int GeneratorUser_AnimationPointClass = RegisterObject< GeneratorUser< types::Animation<types::Point> > >("Generator/Point/Points animation user value").setName("Points animation").setDescription("Lets you store a value for use in other objects");

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

		if(singleValue.getValue() || !m_isInStep)
			removeChilds(xmlRoot);

		QDomElement xmlData = xmlDoc.createElement("SavedData");
		input.save(xmlDoc, xmlData);
		xmlRoot.appendChild(xmlData);
	}

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

int StoreValue_ColorClass = RegisterObject< StoreValue<types::Color> >("File/Color/Save color").setDescription("Save a value in a file for later use");
int StoreValue_DoubleClass = RegisterObject< StoreValue<PReal> >("File/Real/Save real").setDescription("Save a value in a file for later use");
int StoreValue_GradientClass = RegisterObject< StoreValue<types::Gradient> >("File/Gradient/Save gradient").setDescription("Save a value in a file for later use");
int StoreValue_IntegerClass = RegisterObject< StoreValue<int> >("File/Integer/Save integer").setDescription("Save a value in a file for later use");
int StoreValue_MeshClass = RegisterObject< StoreValue<types::Mesh> >("File/Mesh/Save mesh").setDescription("Save a value in a file for later use");
int StoreValue_PathClass = RegisterObject< StoreValue<types::Path> >("File/Path/Save path").setDescription("Save a value in a file for later use");
int StoreValue_PointClass = RegisterObject< StoreValue<types::Point> >("File/Point/Save point").setDescription("Save a value in a file for later use");
int StoreValue_RectClass = RegisterObject< StoreValue<types::Rect> >("File/Rectangle/Save rectangle").setDescription("Save a value in a file for later use");
int StoreValue_ShaderClass = RegisterObject< StoreValue<types::Shader> >("File/Shader/Save shader").setDescription("Save a value in a file for later use");
int StoreValue_StringClass = RegisterObject< StoreValue<QString> >("File/Text/Save text").setDescription("Save a value in a file for later use");

int StoreValue_VectorColorClass = RegisterObject< StoreValue< QVector<types::Color> > >("File/Color/Save colors list").setDescription("Save a value in a file for later use");
int StoreValue_VectorDoubleClass = RegisterObject< StoreValue< QVector<PReal> > >("File/Real/Save reals list").setDescription("Save a value in a file for later use");
int StoreValue_VectorGradientClass = RegisterObject< StoreValue< QVector<types::Gradient> > >("File/Gradient/Save gradients list").setDescription("Save a value in a file for later use");
int StoreValue_VectorIntegerClass = RegisterObject< StoreValue< QVector<int> > >("File/Integer/Save integers list").setDescription("Save a value in a file for later use");
int StoreValue_VectorMeshClass = RegisterObject< StoreValue< QVector<types::Mesh> > >("File/Mesh/Save meshes list").setDescription("Save a value in a file for later use");
int StoreValue_VectorPathClass = RegisterObject< StoreValue< QVector<types::Path> > >("File/Path/Save paths list").setDescription("Save a value in a file for later use");
int StoreValue_VectorPointClass = RegisterObject< StoreValue< QVector<types::Point> > >("File/Point/Save points list").setDescription("Save a value in a file for later use");
int StoreValue_VectorRectClass = RegisterObject< StoreValue< QVector<types::Rect> > >("File/Rectangle/Save rectangles list").setDescription("Save a value in a file for later use");
int StoreValue_VectorShaderClass = RegisterObject< StoreValue< QVector<types::Shader> > >("File/Shader/Save shaders list").setDescription("Save a value in a file for later use");
int StoreValue_VectorStringClass = RegisterObject< StoreValue< QVector<QString> > >("File/Text/Save texts list").setDescription("Save a value in a file for later use");

int StoreValue_AnimationColorClass = RegisterObject< StoreValue< types::Animation<types::Color> > >("File/Color/Save colors animation").setDescription("Save a value in a file for later use");
int StoreValue_AnimationDoubleClass = RegisterObject< StoreValue< types::Animation<PReal> > >("File/Real/Save reals animation").setDescription("Save a value in a file for later use");
int StoreValue_AnimationGradientClass = RegisterObject< StoreValue< types::Animation<types::Gradient> > >("File/Gradient/Save gradients animation").setDescription("Save a value in a file for later use");
int StoreValue_AnimationPointClass = RegisterObject< StoreValue< types::Animation<types::Point> > >("File/Point/Save points animation").setDescription("Save a value in a file for later use");

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

		xmlRoot = doc.documentElement();
		xmlData = xmlRoot.firstChildElement("SavedData");
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
	QDomElement xmlRoot;
	QDomElement xmlData;
};

int LoadValue_ColorClass = RegisterObject< LoadValue<types::Color> >("File/Color/Load color").setDescription("Load a value from a file");
int LoadValue_DoubleClass = RegisterObject< LoadValue<PReal> >("File/Real/Load real").setDescription("Load a value from a file");
int LoadValue_GradientClass = RegisterObject< LoadValue<types::Gradient> >("File/Gradient/Load gradient").setDescription("Load a value from a file");
int LoadValue_IntegerClass = RegisterObject< LoadValue<int> >("File/Integer/Load integer").setDescription("Load a value from a file");
int LoadValue_MeshClass = RegisterObject< LoadValue<types::Mesh> >("File/Mesh/Load mesh").setDescription("Load a value from a file");
int LoadValue_PathClass = RegisterObject< LoadValue<types::Path> >("File/Path/Load path").setDescription("Load a value from a file");
int LoadValue_PointClass = RegisterObject< LoadValue<types::Point> >("File/Point/Load point").setDescription("Load a value from a file");
int LoadValue_RectClass = RegisterObject< LoadValue<types::Rect> >("File/Rectangle/Load rectangle").setDescription("Load a value from a file");
int LoadValue_ShaderClass = RegisterObject< LoadValue<types::Shader> >("File/Shader/Load shader").setDescription("Load a value from a file");
int LoadValue_StringClass = RegisterObject< LoadValue<QString> >("File/Text/Load text").setDescription("Load a value from a file");

int LoadValue_VectorColorClass = RegisterObject< LoadValue< QVector<types::Color> > >("File/Color/Load colors list").setDescription("Load a value from a file");
int LoadValue_VectorDoubleClass = RegisterObject< LoadValue< QVector<PReal> > >("File/Real/Load reals list").setDescription("Load a value from a file");
int LoadValue_VectorGradientClass = RegisterObject< LoadValue< QVector<types::Gradient> > >("File/Gradient/Load gradients list").setDescription("Load a value from a file");
int LoadValue_VectorIntegerClass = RegisterObject< LoadValue< QVector<int> > >("File/Integer/Load integers list").setDescription("Load a value from a file");
int LoadValue_VectorMeshClass = RegisterObject< LoadValue< QVector<types::Mesh> > >("File/Mesh/Load meshes list").setDescription("Load a value from a file");
int LoadValue_VectorPathClass = RegisterObject< LoadValue< QVector<types::Path> > >("File/Path/Load paths list").setDescription("Load a value from a file");
int LoadValue_VectorPointClass = RegisterObject< LoadValue< QVector<types::Point> > >("File/Point/Load points list").setDescription("Load a value from a file");
int LoadValue_VectorRectClass = RegisterObject< LoadValue< QVector<types::Rect> > >("File/Rectangle/Load rectangles list").setDescription("Load a value from a file");
int LoadValue_VectorShaderClass = RegisterObject< LoadValue< QVector<types::Shader> > >("File/Shader/Load shaders list").setDescription("Load a value from a file");
int LoadValue_VectorStringClass = RegisterObject< LoadValue< QVector<QString> > >("File/Text/Load texts list").setDescription("Load a value from a file");

int LoadValue_AnimationColorClass = RegisterObject< LoadValue< types::Animation<types::Color> > >("File/Color/Load colors animation").setDescription("Load a value from a file");
int LoadValue_AnimationDoubleClass = RegisterObject< LoadValue< types::Animation<PReal> > >("File/Real/Load reals animation").setDescription("Load a value from a file");
int LoadValue_AnimationGradientClass = RegisterObject< LoadValue< types::Animation<types::Gradient> > >("File/Gradient/Load gradients animation").setDescription("Load a value from a file");
int LoadValue_AnimationPointClass = RegisterObject< LoadValue< types::Animation<types::Point> > >("File/Point/Load points animation").setDescription("Load a value from a file");

} // namespace Panda

