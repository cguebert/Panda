#include <panda/document/PandaDocument.h>
#include <panda/object/ObjectFactory.h>
#include <panda/object/GenericObject.h>

#include <algorithm>
#include "TinyExpr/tinyexpr.h"

namespace panda {

class DoubleMath_EvalExpression : public GenericObject
{
public:
	PANDA_ABSTRACT_CLASS(DoubleMath_EvalExpression, GenericObject)

	DoubleMath_EvalExpression(PandaDocument *doc)
		: GenericObject(doc)
		, m_expression(initData(std::string("2 + 3^4 / 5 + 6 * 7"), "expression", "The expression to evaluate"))
		, m_debug(initData("debug", "Debug string if the parse fails"))
		, m_generic(initData("input", "Connect here the values of the variables"))
		, m_result(initData("result", "Result of the operation"))
	{
		addInput(m_expression);
		addInput(m_generic);
		addOutput(m_result);

		GenericDataDefinitionList defList;
		int typeOfList = types::DataTypeId::getFullTypeOfVector(0);	// Create a list of the same type as the data connected
		defList.emplace_back(typeOfList,
							 GenericDataDefinition::Input,
							 "input",
							 "List from which to extract values");

		int stringType = types::DataTypeId::getFullTypeOfSingleValue(types::DataTypeId::getIdOf<std::string>());
		defList.emplace_back(stringType,
							 GenericDataDefinition::Input,
							 "name",
							 "Name of the variable");

		using floatValues = std::tuple<float>;
		setupGenericObject<floatValues>(this, m_generic, defList);
	}

	void update()
	{
		m_variables.clear();
		const auto expression = m_expression.getValue();
		auto acc = m_result.getAccessor();
		acc.clear();

		if (expression.empty())
		{
			m_debug.setValue("");
			return;
		}

		GenericObject::update();

		const auto exprStr = expression.c_str();
		int error = 0;

		if (m_variables.empty())
		{
			auto val = te_interp(exprStr, &error);
			acc.push_back(static_cast<float>(val));
		}
		else
		{
			int nbVar = m_variables.size();
			std::vector<double> values(nbVar, 0);
			std::vector<te_variable> vars;
			
			// Prepare the variables and compute the number of outputs
			int valLen = m_variables[0].nb;
			int maxLen = valLen;
			for (int i = 0; i < nbVar; ++i)
			{
				const auto& v = m_variables[i];
				vars.push_back({ v.name.c_str(), &values[i] });

				int len = v.nb;
				if (!len)
				{
					valLen = 0;
					break;
				}
				else if (len != 1 && len != valLen && len != maxLen)
				{
					if (maxLen == 1)
						valLen = maxLen = len;
					else
						valLen = 1;
				}
			}

			// Compile and compute the values
			if (valLen)
			{
				auto comp = te_compile(exprStr, vars.data(), nbVar, &error);
				if (comp)
				{
					for (int i = 0; i < valLen; ++i)
					{
						for (int j = 0; j < nbVar; ++j)
							values[j] = (*m_variables[j].values)[i % m_variables[j].nb];
						acc.push_back(static_cast<float>(te_eval(comp)));
					}
					te_free(comp);
				}
			}
		}

		// Debug string if there was an error
		if (error)
		{
			--error;
			int pos = std::max(0, error - 3);
			int len = error - pos;
			m_debug.setValue("Error at: " + expression.substr(pos, len) 
				+ ">" + expression.substr(error, 1)
				+ "<" + expression.substr(error + 1, 3));
		}
		else
			m_debug.setValue("");
	}

	template <class T>
	void updateT(DataList& list)
	{
		typedef Data< std::vector<T> > ListData;
		typedef Data< std::string > StringData;
		ListData* dataInput = dynamic_cast<ListData*>(list[0]);
		StringData* dataName = dynamic_cast<StringData*>(list[1]);
		
		assert(dataInput && dataName);

		VariableData var;
		var.name = dataName->getValue();
		var.values = &dataInput->getValue();
		var.nb = var.values->size();
		m_variables.push_back(var);
	}

	virtual float compute(const float& value) { return 0; }

protected:
	Data< std::string > m_expression, m_debug;
	GenericVectorData m_generic;
	Data< std::vector<float> > m_result;
	
	struct VariableData
	{
		int nb = 0;
		const std::vector<float>* values;
		std::string name;
	};
	std::vector<VariableData> m_variables;
};

int DoubleMath_EvalExpressionClass = RegisterObject<DoubleMath_EvalExpression>("Math/Real/Eval expression").setDescription("Evaluate a custom expression with unlimited inputs");

} // namespace Panda


