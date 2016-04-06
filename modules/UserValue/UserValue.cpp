#include <panda/object/ObjectFactory.h>

#include <panda/helper/typeList.h>
#include <panda/types/Animation.h>
#include <panda/types/DataTraits.h>
#include <panda/types/typesLists.h>

#include "UserValue.h"

#include <ctype.h>

namespace
{
	inline std::string capitalized(const std::string& s)
	{
		auto ret = s;
		ret[0] = toupper(ret[0]);
		return ret;
	}

	struct RegisterSingleValue
	{
		template <class T> void operator()(T)
		{
			auto typeName = capitalized(panda::types::DataTraitsList::getTraitOf<T>()->valueTypeName());
			auto menuName = "Generator/" + typeName + "/" + typeName + " user value";
			auto name = typeName + " value";

			int r = panda::RegisterObject<panda::GeneratorUser<T>>(menuName)
				.setName(name)
				.setDescription("Lets you store a value for use in other objects");
		}
	};

	struct RegisterVectors
	{
		template <class T> void operator()(T)
		{
			auto trait = panda::types::DataTraitsList::getTraitOf<T>();
			auto typeName = capitalized(trait->valueTypeName());
			auto plural = capitalized(trait->valueTypeNamePlural());
			auto menuName = "Generator/" + typeName + "/" + plural + " list user value";
			auto name = typeName + " list";

			int r = panda::RegisterObject<panda::GeneratorUser<std::vector<T>>>(menuName)
				.setName(name)
				.setDescription("Lets you store a value for use in other objects");
		}
	};

	struct RegisterAnimations
	{
		template <class T> void operator()(T)
		{
			auto trait = panda::types::DataTraitsList::getTraitOf<T>();
			auto typeName = capitalized(trait->valueTypeName());
			auto plural = capitalized(trait->valueTypeNamePlural());
			auto menuName = "Generator/" + typeName + "/" + plural + " animation user value";
			auto name = typeName + " animation";

			int r = panda::RegisterObject<panda::GeneratorUser<panda::types::Animation<T>>>(menuName)
				.setName(name)
				.setDescription("Lets you store a value for use in other objects");
		}
	};
}

namespace panda 
{

	int RegisterUserValueObjects()
	{
		helper::for_each_type<allDataTypes>(RegisterSingleValue());
		helper::for_each_type<allDataTypes>(RegisterVectors());
		helper::for_each_type<allAnimationTypes>(RegisterAnimations());
		return 1;
	}
	
	int GeneratorUserValue_Classes = RegisterUserValueObjects();

} // namespace Panda

panda::ModuleHandle userValueModule = REGISTER_MODULE
		.setDescription("Object to store values for the UI or to & from files.")
		.setLicense("GPL")
		.setVersion("1.0");
