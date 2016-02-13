#include <panda/PandaDocument.h>
#include <panda/XmlDocument.h>
#include <panda/SimpleGUI.h>
#include <panda/TimedFunctions.h>

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
	}

	void preDestruction() override
	{
		if (TimedFunctions::instance().cancelRun(m_saveTimerId))
			saveToFile();
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

		if (m_singleValue.getValue() || !isInStep())
			initRoot();

		auto xmlData = m_xmlRoot.addChild("SavedData");
		m_input.save(xmlData);
	}

	void endStep()
	{
		addValue();
		PandaObject::endStep();

		TimedFunctions::instance().cancelRun(m_saveTimerId);
		m_saveTimerId = TimedFunctions::instance().delayRun(0.5, [this]() { 
			parentDocument()->getGUI().executeByUI([this](){
				saveToFile();
			});
		});
	}

	void saveToFile()
	{
		auto tmpFileName = m_fileName.getValue();
		if(tmpFileName.empty())
			return;

		m_xmlDoc.saveToFile(tmpFileName);
	}

protected:
	Data<T> m_input;
	Data<std::string> m_fileName;
	Data<int> m_singleValue;
	XmlDocument m_xmlDoc;
	XmlElement m_xmlRoot;
	int m_saveTimerId = -1;
};

} // namespace Panda
