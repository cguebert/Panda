#include <panda/PandaObject.h>
#include <panda/XmlDocument.h>

#include <QTimer>

namespace panda {

template <class T>
class StoreValue : public PandaObject
{
public:
	PANDA_CLASS(PANDA_TEMPLATE(StoreValue, T), PandaObject)

	StoreValue(PandaDocument *doc)
		: PandaObject(doc)
		, m_input(initData("input", "The value you want to store"))
		, m_fileName(initData("file name", "File where to store the value"))
		, m_singleValue(initData(1, "single value", "If false save all the values during the animation"))
	{
		addInput(m_input);
		addInput(m_fileName);
		addInput(m_singleValue);

		m_fileName.setWidget("save file");
		m_singleValue.setWidget("checkbox");

		m_saveTimer.setSingleShot(true);

		QObject::connect(&m_saveTimer, &QTimer::timeout, [this]() { onTimeout(); });
	}

	void initRoot()
	{
		m_xmlDoc.clear();
		m_xmlRoot = m_xmlDoc.root();
		m_xmlRoot.setName("PandaValue");
	}

	void reset()
	{
		initRoot();
	}

	void addValue()
	{
		if(!m_xmlRoot)
			initRoot();

		if (m_singleValue.getValue() || !m_isInStep)
			initRoot();

		auto xmlData = m_xmlRoot.addChild("SavedData");
		m_input.save(xmlData);
	}

	void endStep()
	{
		addValue();
		PandaObject::endStep();

		m_saveTimer.stop();
		m_saveTimer.start(500);
	}

	void saveToFile()
	{
		auto tmpFileName = m_fileName.getValue();
		if(tmpFileName.empty())
			return;

		m_xmlDoc.saveToFile(tmpFileName);
	}

	void onTimeout()
	{
		saveToFile();
	}

protected:
	Data<T> m_input;
	Data<std::string> m_fileName;
	Data<int> m_singleValue;
	XmlDocument m_xmlDoc;
	XmlElement m_xmlRoot;
	QTimer m_saveTimer;
};

} // namespace Panda
