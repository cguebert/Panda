#include <panda/PandaDocument.h>
#include <panda/object/PandaObject.h>
#include <panda/object/ObjectFactory.h>

#include <chrono>
#include <iomanip>
#include <sstream>

namespace panda {

class GeneratorText_DateTime : public PandaObject
{
public:
	PANDA_CLASS(GeneratorText_DateTime, PandaObject)

	GeneratorText_DateTime(PandaDocument *doc)
		: PandaObject(doc)
		, format(initData(std::string("%H:%M:%S"), "format", "Format used to create the text"))
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
			auto now = std::chrono::system_clock::now();
			auto nowPlusDelta = now + std::chrono::milliseconds(delta.getValue());
			auto date = std::chrono::system_clock::to_time_t(nowPlusDelta);
			if(date != previousDate)
				PandaObject::setDirtyValue(caller);
		}
	}

	void update()
	{
		auto now = std::chrono::system_clock::now();
		auto nowPlusDelta = now + std::chrono::milliseconds(delta.getValue());
		auto toTime = std::chrono::system_clock::to_time_t(nowPlusDelta);
		previousDate = toTime;

		auto tm = std::localtime(&toTime);

		std::ostringstream ss;
		ss << std::put_time(tm, format.getValue().c_str());
		
		text.setValue(ss.str());

		cleanDirty();
	}

protected:
	Data<std::string> format, text;
	Data<int> delta;
	std::time_t previousDate = 0;
};

int GeneratorText_DateTimeClass = RegisterObject<GeneratorText_DateTime>("Generator/Text/Date & time").setDescription("Create a text using the current date");

} // namespace Panda
