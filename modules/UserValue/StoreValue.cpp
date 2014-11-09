#include <panda/PandaDocument.h>
#include <panda/PandaObject.h>
#include <panda/ObjectFactory.h>
#include <panda/Group.h>

#include <panda/types/Animation.h>
#include <panda/types/Color.h>
#include <panda/types/Gradient.h>
#include <panda/types/Mesh.h>
#include <panda/types/Path.h>
#include <panda/types/Rect.h>
#include <panda/types/Shader.h>

#include "StoreValue.h"

#include <QString>
#include <QVector>

#include <QTimer>
#include <QFile>
#include <QDomDocument>

namespace panda {

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


} // namespace Panda