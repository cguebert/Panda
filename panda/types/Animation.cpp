#include <panda/types/Animation.inl>
#include <panda/types/AnimationTraits.h>

#include <panda/DataFactory.h>
#include <panda/Data.inl>

namespace panda
{

namespace types
{

template class Animation<PReal>;
template class Data< Animation<PReal> >;
int realAnimationDataClass = RegisterData< Animation<PReal> >();

} // namespace types

} // namespace panda
