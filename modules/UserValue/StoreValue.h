#include <panda/PandaObject.h>

#include <QTimer>
#include <QFile>

namespace panda {

template <class T>
class StoreValue : public PandaObject
{
public:
	PANDA_CLASS(PANDA_TEMPLATE(StoreValue, T), PandaObject)

	StoreValue(PandaDocument *doc)
		: PandaObject(doc)
		, input(initData("input", "The value you want to store"))
		, fileName(initData("file name", "File where to store the value"))
		, singleValue(initData(1, "single value", "If false save all the values during the animation"))
	{
		addInput(input);
		addInput(fileName);
		addInput(singleValue);

		fileName.setWidget("save file");
		singleValue.setWidget("checkbox");

		saveTimer.setSingleShot(true);

		QObject::connect(&saveTimer, &QTimer::timeout, [this]() { onTimeout(); });
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

		saveTimer.stop();
		saveTimer.start(500);
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
	QTimer saveTimer;
};

} // namespace Panda
