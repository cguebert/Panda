#include <panda/PandaObject.h>
#include <panda/Group.h> // For BaseGeneratorUser
#include <panda/XmlDocument.h>

namespace panda {

template <class T>
class CustomData : public Data<T>
{
public:
	PANDA_CLASS(PANDA_TEMPLATE(CustomData, T), PANDA_TEMPLATE(Data, T))

	CustomData(const std::string& name, const std::string& help, PandaObject* owner)
		: Data<T>(name, help, owner)
	{ }
	virtual void save(XmlElement& elem) const override
	{
		Data<T>::save(elem);
		std::string w = getWidget();
		if(w != "default")
			elem.setAttribute("widget", w);
		std::string d = getWidgetData();
		if(!d.empty())
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
		, m_userValue("input", "The value you want to store", this)
		, m_output(initData("value", "The value stored"))
	{
		addInput(m_userValue);
		addOutput(m_output);

		m_output.setDisplayed(false);

		dataSetParent(&m_output, &m_userValue);
	}

	bool hasConnectedInput()
	{ return m_userValue.getParent(); }

	bool hasConnectedOutput()
	{ return !m_output.getOutputs().empty(); }

	BaseData* getInputUserData()
	{ return &m_userValue; }

	BaseData* getOutputUserData()
	{ return &m_output; }

	void save(XmlDocument& elem, const QList<PandaObject*> *selected)
	{	// Compared to PandaObject::save, we want to always save the userValue, because of the customData (widget & widgetData)
		for(BaseData* data : m_datas)
		{
			if(data == &m_userValue ||
					(data->isSet() && data->isPersistent() && !data->isReadOnly()
					&& !(selected && data->getParent() && selected->contains(data->getParent()->getOwner()))))
			{
				auto xmlData = elem.addChild("Data");
				xmlData.setAttribute("name", data->getName());
				data->save(doc, xmlData);
			}
		}
	}

protected:
	CustomData<T> m_userValue;
	Data<T> m_output;
};

} // namespace Panda
