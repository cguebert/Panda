#include <panda/document/PandaDocument.h>
#include <panda/object/GenericObject.h>
#include <panda/object/ObjectFactory.h>
#include <panda/helper/algorithm.h>

#include <boost/format.hpp>

namespace
{

template <class T> std::string convert(const T& value)
{ return std::to_string(value); }

std::string convert(const std::string& value)
{ return value; }

}

namespace panda {

typedef std::tuple<int, float, std::string> formatTypes;

class GeneratorText_Format : public GenericObject
{
public:
	PANDA_CLASS(GeneratorText_Format, GenericObject)

	GeneratorText_Format(PandaDocument *doc)
		: GenericObject(doc)
		, m_formatString(initData("format", "Format used to create the text"))
		, m_text(initData("text", "Text obtained by the format operation"))
		, m_generic(initData("input", "Connect here the values to use in the format operation"))
	{
		addInput(m_formatString);
		addInput(m_generic);

		addOutput(m_text);

		int typeOfValue = types::DataTypeId::getFullTypeOfVector(0);	// Create a copy of the data connected
		GenericDataDefinitionList defList;
		defList.emplace_back(typeOfValue,
							 GenericDataDefinition::Input,
							 "input",
							 "Value to use in the format operation");
		setupGenericObject<formatTypes>(this, m_generic, defList);
	}

	void update()
	{
		auto output = m_text.getAccessor();
		output.clear();
		m_formatters.clear();

		try
		{
			auto formatter = boost::format(m_formatString.getValue());
			formatter.exceptions(boost::io::no_error_bits);
			m_formatters.push_back(formatter);
		}
		catch (...)
		{
			return;
		}

		GenericObject::update();

		for (const auto& f : m_formatters)
			output.push_back(f.str());
	}

	template <class T>
	void updateT(DataList& list)
	{
		if (m_error)
			return;

		typedef Data< std::vector<T> > ValueData;
		ValueData* dataInput = dynamic_cast<ValueData*>(list[0]);
		assert(dataInput);

		const std::vector<T>& inVal = dataInput->getValue();
		int nb = inVal.size();
		int prevNb = m_formatters.size();

		if(prevNb == 1 && nb > 1) // Expand the list, if there was only one
			m_formatters.resize(nb, m_formatters[0]);
		else if(nb == 1 && prevNb > 1) // Use this value for each formatter in the list
			nb = 1;
		else // Multiple list of various sizes, for now take the minimum size
			m_formatters.resize(std::min(prevNb, nb));
		int size = m_formatters.size();

		for (int i = 0; i < size; ++i)
			m_formatters[i] % inVal[i % nb];
	}

protected:
	Data<std::string> m_formatString;
	Data< std::vector<std::string> > m_text;
	GenericVectorData m_generic;

	bool m_error = false;
	std::vector<boost::format> m_formatters;
};

int GeneratorText_FormatClass = RegisterObject<GeneratorText_Format>("Generator/Text/Format").setDescription("Create a text by replacing markers by input values");

} // namespace Panda
