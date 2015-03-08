#include <panda/PandaObject.h>
#include <panda/Group.h> // For BaseGeneratorUser

#include <QFile>

namespace panda {

template <class T>
class CustomData : public Data<T>
{
public:
	PANDA_CLASS(PANDA_TEMPLATE(CustomData, T), PANDA_TEMPLATE(Data, T))

	CustomData(const QString& name, const QString& help, PandaObject* owner)
		: Data<T>(name, help, owner)
	{ }
	virtual void save(QDomDocument& doc, QDomElement& elem) const override
	{
		Data<T>::save(doc, elem);
		QString w = getWidget();
		if(w != "default")
			elem.setAttribute("widget", w);
		QString d = getWidgetData();
		if(!d.isEmpty())
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
	{
		return m_userValue.getParent();
	}

	bool hasConnectedOutput()
	{
		return !m_output.getOutputs().empty();
	}

	BaseData* getInputUserData()
	{
		return &m_userValue;
	}

	BaseData* getOutputUserData()
	{
		return &m_output;
	}

	void save(QDomDocument& doc, QDomElement& elem, const QList<PandaObject*> *selected)
	{	// Compared to PandaObject::save, we want to always save the userValue, because of the customData (widget & widgetData)
		for(BaseData* data : m_datas)
		{
			if(data == &m_userValue ||
					(data->isSet() && data->isPersistent() && !data->isReadOnly()
					&& !(selected && data->getParent() && selected->contains(data->getParent()->getOwner()))))
			{
				QDomElement xmlData = doc.createElement("Data");
				xmlData.setAttribute("name", data->getName());
				data->save(doc, xmlData);
				elem.appendChild(xmlData);
			}
		}
	}

protected:
	CustomData<T> m_userValue;
	Data<T> m_output;
};

} // namespace Panda
