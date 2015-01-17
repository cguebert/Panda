#include <panda/PandaObject.h>
#include <panda/Group.h> // For BaseGeneratorUser

#include <QFile>

namespace panda {

template <class T>
class CustomData : public Data<T>
{
public:
	CustomData(const QString& name, const QString& help, PandaObject* owner)
		: Data<T>(name, help, owner)
	{ }
	virtual void save(QDomDocument& doc, QDomElement& elem)
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
		, userValue("input", "The value you want to store", this)
		, output(initData(&output, "value", "The value stored"))
	{
		addInput(userValue);
		addOutput(output);

		output.setDisplayed(false);

		dataSetParent(&output, &userValue);
	}

	bool hasConnectedInput()
	{
		return userValue.getParent();
	}

	bool hasConnectedOutput()
	{
		return !output.getOutputs().empty();
	}

	BaseData* getInputUserData()
	{
		return &userValue;
	}

	BaseData* getOutputUserData()
	{
		return &output;
	}

protected:
	CustomData<T> userValue;
	Data<T> output;
};

} // namespace Panda
