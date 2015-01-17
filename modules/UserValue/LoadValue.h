#include <panda/PandaObject.h>

#include <QFile>

namespace panda {

template <class T>
class LoadValue : public PandaObject
{
public:
	PANDA_CLASS(PANDA_TEMPLATE(LoadValue, T), PandaObject)

	LoadValue(PandaDocument *doc)
		: PandaObject(doc)
		, m_output(initData(&m_output, "output", "The loaded value"))
		, m_fileName(initData(&m_fileName, "file name", "File where to read the value"))
	{
		addInput(m_fileName);
		m_fileName.setWidget("open file");

		addOutput(m_output);
	}

	void reset()
	{
		QString tmpFileName = m_fileName.getValue();
		if(tmpFileName.isEmpty())
			return;
		QFile file(tmpFileName);
		if (!file.open(QIODevice::ReadOnly))
			return;

		QDomDocument doc;
		int errLine, errCol;
		if (!doc.setContent(&file, nullptr, &errLine, &errCol))
			return;

		m_xmlRoot = doc.documentElement();
		m_xmlData = m_xmlRoot.firstChildElement("SavedData");
	}

	void beginStep()
	{
		PandaObject::beginStep();

		if(!m_xmlData.isNull())
		{
			m_output.load(m_xmlData);

			m_xmlData = m_xmlData.nextSiblingElement("SavedData");
		}
	}

protected:
	Data<T> m_output;
	Data<QString> m_fileName;
	QDomElement m_xmlRoot, m_xmlData;
};

} // namespace Panda
