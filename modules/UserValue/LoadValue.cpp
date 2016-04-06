#include <panda/object/ObjectFactory.h>

#include <panda/helper/typeList.h>
#include <panda/types/Animation.h>
#include <panda/types/DataTraits.h>
#include <panda/types/typesLists.h>

#include "LoadValue.h"

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
			auto trait = panda::types::DataTraitsList::getTraitOf<T>();
			auto typeName = trait->valueTypeName();
			auto cap = capitalized(typeName);
			auto menuName = "File/" + cap + "/Load " + typeName;

			int r = panda::RegisterObject<panda::LoadValue<T>>(menuName)
				.setDescription("Load a value from a file");
		}
	};

	struct RegisterVectors
	{
		template <class T> void operator()(T)
		{
			auto trait = panda::types::DataTraitsList::getTraitOf<T>();
			auto menuName = "File/" + capitalized(trait->valueTypeName()) + "/Load " + trait->valueTypeNamePlural() + " list";

			int r = panda::RegisterObject<panda::LoadValue<std::vector<T>>>(menuName)
				.setDescription("Load a value from a file");
		}
	};

	struct RegisterAnimations
	{
		template <class T> void operator()(T)
		{
			auto trait = panda::types::DataTraitsList::getTraitOf<T>();
			auto menuName = "File/" + capitalized(trait->valueTypeName()) + "/Load " + trait->valueTypeNamePlural() + " animation";

			int r = panda::RegisterObject<panda::LoadValue<panda::types::Animation<T>>>(menuName)
				.setDescription("Load a value from a file");
		}
	};
}

namespace panda 
{

	int RegisterLoadValueObjects()
	{
		helper::for_each_type<allDataTypes>(RegisterSingleValue());
		helper::for_each_type<allDataTypes>(RegisterVectors());
		helper::for_each_type<allAnimationTypes>(RegisterAnimations());
		return 1;
	}

	int GeneratorLoadValue_Classes = RegisterLoadValueObjects();

} // namespace Panda
