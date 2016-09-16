#include <panda/object/PandaObject.h>
#include <panda/object/Group.h> // For BaseGeneratorUser
#include <panda/XmlDocument.h>
#include <panda/helper/algorithm.h>

namespace panda {

template <class T>
class GeneratorUser : public BaseGeneratorUser
{
public:
	PANDA_CLASS(PANDA_TEMPLATE(GeneratorUser, T), BaseGeneratorUser)

	GeneratorUser(PandaDocument *doc)
		: BaseGeneratorUser(doc)
		, m_userValue("input", "The value you want to store", this)
		, m_output(initData("value", "The value stored"))
	{
		addInput(m_userValue);
		addOutput(m_output);

		m_output.setDisplayed(false);

		dataSetParent(&m_output, &m_userValue);
	}

	bool hasConnectedInput()
	{ return m_userValue.getParent() != nullptr; }

	bool hasConnectedOutput()
	{ return !m_output.getOutputs().empty(); }

	BaseData* getInputUserData()
	{ return &m_userValue; }

	BaseData* getOutputUserData()
	{ return &m_output; }

	void save(XmlElement& elem, const std::vector<PandaObject*> *selected) override
	{	// Compared to PandaObject::save, we want save the userValue custom parameters (widget & widgetData)
		for(BaseData* data : getDatas())
		{
			XmlElement xmlData;
			if (data == &m_userValue) // Save the custom parameters of the user data, if not default
			{
				std::string widget = data->getWidget();
				std::string widgetData = data->getWidgetData();
				if (widget != "default" || !widgetData.empty())
				{
					xmlData = elem.addChild("Data");
					xmlData.setAttribute("name", data->getName());

					if(widget != "default")
						xmlData.setAttribute("widget", widget);
				
					if(!widgetData.empty())
						xmlData.setAttribute("widgetData", widgetData);
				}
			}

			// Do we save the value ?
			if(data->isSet() && data->isPersistent() && !data->isReadOnly()
				&& !(selected && data->getParent() && helper::contains(*selected, data->getParent()->getOwner())))
			{
				if (!xmlData)
				{
					xmlData = elem.addChild("Data");
					xmlData.setAttribute("name", data->getName());
				}
				data->save(xmlData);
			}
		}
	}

protected:
	Data<T> m_userValue;
	Data<T> m_output;
};

} // namespace Panda
