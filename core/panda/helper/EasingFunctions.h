#ifndef HELPER_EASINGFUNCTIONS_H
#define HELPER_EASINGFUNCTIONS_H

#include <panda/core.h>

#include <string>
#include <vector>

namespace panda
{

namespace helper
{

class PANDA_CORE_API EasingFunctions
{
public:
	enum class Type
	{
		Linear,
		InQuad, OutQuad, InOutQuad, OutInQuad,
		InCubic, OutCubic, InOutCubic, OutInCubic,
        InQuart, OutQuart, InOutQuart, OutInQuart,
        InQuint, OutQuint, InOutQuint, OutInQuint,
        InSine, OutSine, InOutSine, OutInSine,
        InExpo, OutExpo, InOutExpo, OutInExpo,
        InCirc, OutCirc, InOutCirc, OutInCirc,
        InBack, OutBack, InOutBack, OutInBack
	};

	static const std::vector<std::string>& TypeNames();

	EasingFunctions();

	Type type() const;
	void setType(Type type);

	float valueForProgress(float progress) const;

private:
	Type m_type = Type::Linear;
	using EasingFunc = float(*)(float);
	EasingFunc m_func;
};

inline EasingFunctions::Type EasingFunctions::type() const
{ return m_type; }

} // namespace helper

} // namespace panda


#endif // HELPER_EASINGFUNCTIONS_H
