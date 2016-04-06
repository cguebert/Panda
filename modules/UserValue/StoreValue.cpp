#include <panda/object/ObjectFactory.h>

#include <panda/helper/typeList.h>
#include <panda/types/Animation.h>
#include <panda/types/DataTraits.h>
#include <panda/types/typesLists.h>

#include "StoreValue.h"

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
			auto menuName = "File/" + cap + "/Save " + typeName;

			int r = panda::RegisterObject<panda::StoreValue<T>>(menuName)
				.setDescription("Save a value in a file for later use");
		}
	};

	struct RegisterVectors
	{
		template <class T> void operator()(T)
		{
			auto trait = panda::types::DataTraitsList::getTraitOf<T>();
			auto menuName = "File/" + capitalized(trait->valueTypeName()) + "/Save " + trait->valueTypeNamePlural() + " list";

			int r = panda::RegisterObject<panda::StoreValue<std::vector<T>>>(menuName)
				.setDescription("Save a value in a file for later use");
		}
	};

	struct RegisterAnimations
	{
		template <class T> void operator()(T)
		{
			auto trait = panda::types::DataTraitsList::getTraitOf<T>();
			auto menuName = "File/" + capitalized(trait->valueTypeName()) + "/Save " + trait->valueTypeNamePlural() + " animation";

			int r = panda::RegisterObject<panda::StoreValue<panda::types::Animation<T>>>(menuName)
				.setDescription("Save a value in a file for later use");
		}
	};
}

namespace panda 
{

	int RegisterStoreValueObjects()
	{
		helper::for_each_type<allDataTypes>(RegisterSingleValue());
		helper::for_each_type<allDataTypes>(RegisterVectors());
		helper::for_each_type<allAnimationTypes>(RegisterAnimations());
		return 1;
	}

	int GeneratorStoreValue_Classes = RegisterStoreValueObjects();

} // namespace Panda
