#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>
#include <QDateTime>

namespace panda {

class GeneratorText_DateTime : public PandaObject
{
public:
	PANDA_CLASS(GeneratorText_DateTime, PandaObject)

	GeneratorText_DateTime(PandaDocument *doc)
		: PandaObject(doc)
		, format(initData(std::string("hh:mm:ss"), "format", "Format used to create the text"))
		, text(initData("text", "Text containing the date & time using the format"))
		, delta(initData("delta", "Delta in milliseconds added to the current time"))
	{
		addInput(format);
		addInput(delta);

		addOutput(text);

		BaseData* docTime = doc->getData("time");
		if(docTime)
			addInput(*docTime);
	}

	void setDirtyValue(const DataNode* caller)
	{
		if(!m_dirtyValue)
		{
			QDateTime date = QDateTime::currentDateTime();
			date.addMSecs(delta.getValue());

			if(date != previousDate)
				PandaObject::setDirtyValue(caller);
		}
	}

	void update()
	{
		QDateTime date = QDateTime::currentDateTime();
		date.addMSecs(delta.getValue());
		previousDate = date;

		text.setValue(date.toString(QString::fromStdString(format.getValue())).toStdString());

		cleanDirty();
	}

protected:
	Data<std::string> format, text;
	Data<int> delta;
	QDateTime previousDate;
};

int GeneratorText_DateTimeClass = RegisterObject<GeneratorText_DateTime>("Generator/Text/Date & time").setDescription("Create a text using the current date");

} // namespace Panda
