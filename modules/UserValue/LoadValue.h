#include <panda/document/PandaDocument.h>
#include <panda/XmlDocument.h>

namespace panda {

template <class T>
class LoadValue : public PandaObject
{
public:
	PANDA_CLASS(PANDA_TEMPLATE(LoadValue, T), PandaObject)

	LoadValue(PandaDocument *doc)
		: PandaObject(doc)
		, m_output(initData("output", "The loaded value"))
		, m_fileName(initData("file name", "File where to read the value"))
	{
		addInput(m_fileName);
		m_fileName.setWidget("open file");

		addOutput(m_output);
	}

	void reset()
	{
		m_xmlData = XmlElement();
		if (!m_xmlDoc.loadFromFile(m_fileName.getValue()))
			return;

		m_xmlData = m_xmlDoc.root().firstChild("SavedData");
	}

	void beginStep()
	{
		PandaObject::beginStep();

		if(m_xmlData)
		{
			m_output.load(m_xmlData);
			m_xmlData = m_xmlData.nextSibling("SavedData");
		}
	}

protected:
	Data<T> m_output;
	Data<std::string> m_fileName;
	XmlDocument m_xmlDoc;
	XmlElement m_xmlData;
};

} // namespace Panda
