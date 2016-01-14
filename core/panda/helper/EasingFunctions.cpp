#include <panda/helper/EasingFunctions.h>

/*
Disclaimer for Robert Penner's Easing Equations license:

TERMS OF USE - EASING EQUATIONS

Open source under the BSD License.

Copyright © 2001 Robert Penner
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the author nor the names of contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <math.h>

namespace functions
{
	const float pi = 3.14159265358979323846f;
	const float pi2 = pi / 2.f;
	const float overshoot = 1.70158f;

	float easeLinear(float x)
	{
		return x;
	}

	float easeInQuad(float x)
	{
		return x * x;
	}

	float easeOutQuad(float x)
	{
		return -x * (x - 2.f);
	}

	float easeInOutQuad(float x)
	{
		x *= 2.f;
		if (x < 1.f)
			return x*x / 2.f;

		x -= 1.0f;
		return -0.5f * (x * (x - 2.f) - 1.f);
	}

	float easeOutInQuad(float x)
	{
		if (x < 0.5f)
			return easeOutQuad(x * 2.f) / 2.f;
		return easeInQuad(2.f * x - 1.f) / 2.f + 0.5f;
	}

	float easeInCubic(float x)
	{
		return x*x*x;
	}

	float easeOutCubic(float x)
	{
		x -= 1.f;
		return x*x*x + 1.f;
	}

	float easeInOutCubic(float x)
	{
		x *= 2.f;
		if (x < 1.f)
			return 0.5f * x*x*x;
		x -= 2.f;
		return 0.5f * (x*x*x + 2.f);
	}

	float easeOutInCubic(float x)
	{
		if (x < 0.5f)
			return easeOutCubic(x * 2.f) / 2.f;
		return easeInCubic(2.f * x - 1.f) / 2.f + 0.5f;
	}

	float easeInQuart(float x)
	{
		return x*x*x*x;
	}

	float easeOutQuart(float x)
	{
		x -= 1.f;
		return -(x*x*x*x - 1.f);
	}

	float easeInOutQuart(float x)
	{
		x *= 2.f;
		if (x < 1.f)
			return 0.5f * x*x*x*x;
		x -= 2.f;
		return -0.5f * (x*x*x*x - 2.f);
	}

	float easeOutInQuart(float x)
	{
		if (x < 0.5f)
			return easeOutQuart(x * 2.f) / 2.f;
		return easeInQuart(2.f * x - 1.f) / 2.f + 0.5f;
	}

	float easeInQuint(float x)
	{
		return x*x*x*x*x;
	}

	float easeOutQuint(float x)
	{
		x -= 1.f;
		return x*x*x*x*x + 1.f;
	}

	float easeInOutQuint(float x)
	{
		x *= 2.f;
		if (x < 1.f)
			return 0.5f * x*x*x*x*x;
		x -= 2.f;
		return 0.5f * (x*x*x*x*x + 2.f);
	}

	float easeOutInQuint(float x)
	{
		if (x < 0.5f)
			return easeOutQuint(x * 2.f) / 2.f;
		return easeInQuint(2.f * x - 1.f) / 2.f + 0.5f;
	}

	float easeInSine(float x)
	{
		return (x == 1.f) ? 1.f : -std::cos(x * pi2) + 1.f;
	}

	float easeOutSine(float x)
	{
		return std::sin(x * pi2);
	}

	float easeInOutSine(float x)
	{
		return -0.5f * (std::cos(x * pi) - 1.f);
	}

	float easeOutInSine(float x)
	{
		if (x < 0.5f)
			return easeOutSine(x * 2.f) / 2.f;
		return easeInSine(2.f * x - 1.f) / 2.f + 0.5f;
	}

	float easeInExpo(float x)
	{
		return (x == 0.f || x == 1.f) ? x : std::pow(2.f, 10.f * (x - 1.f)) - 0.001f;
	}

	float easeOutExpo(float x)
	{
		return (x == 1.f) ? 1.f : 1.001f * (-std::pow(2.f, -10.f * x) + 1.f);
	}

	float easeInOutExpo(float x)
	{
		if (x == 0.f || x == 1.f)
			return x;
		x *= 2.f;
		if (x < 1.f)
			return 0.5f * std::pow(2.f, 10.f * (x - 1.f)) - 0.0005f;
		return 0.5f * 1.0005f * (-std::pow(2.f, -10.f * (x - 1.f)) + 2.f);
	}

	float easeOutInExpo(float x)
	{
		if (x < 0.5f)
			return easeOutExpo(x * 2.f) / 2.f;
		return easeInExpo(2.f * x - 1.f) / 2.f + 0.5f;
	}

	float easeInCirc(float x)
	{
		return -(std::sqrt(1.f - x*x) - 1.f);
	}

	float easeOutCirc(float x)
	{
		x -= 1.f;
		return std::sqrt(1 - x*x);
	}

	float easeInOutCirc(float x)
	{
		x *= 2.f;
		if (x < 1.f)
			return -0.5f * (std::sqrt(1.f - x*x) - 1.f);
		x -= 2.f;
		return 0.5f * (std::sqrt(1.f - x*x) + 1.f);
	}

	float easeOutInCirc(float x)
	{
		if (x < 0.5f)
			return easeOutCirc(x * 2.f) / 2.f;
		return easeInCirc(2.f * x - 1.f) / 2.f + 0.5f;
	}

	float easeInBack(float x)
	{
		return x*x * ((overshoot + 1.f) * x - overshoot);
	}

	float easeOutBack(float x)
	{
		x -= 1.f;
		return x*x * ((overshoot + 1.f) * x + overshoot) + 1.f;
	}

	float easeInOutBack(float x)
	{
		x *= 2.f;
		float s = overshoot * 1.525f;
		if (x < 1.f)
			return 0.5f * (x*x * ((s + 1.f) * x - s));
		x -= 2.f;
		return 0.5f * (x*x * ((s + 1.f) * x + s) + 2.f);
	}

	float easeOutInBack(float x)
	{
		if (x < 0.5f)
			return easeOutBack(x * 2.f) / 2.f;
		return easeInBack(2.f * x - 1.f) / 2.f + 0.5f;
	}
}

namespace panda
{

namespace helper
{

const std::vector<std::string>& EasingFunctions::TypeNames() 
{
	static std::vector<std::string> names = 
	{ 
		"Linear",
		"InQuad", "OutQuad", "InOutQuad", "OutInQuad",
		"InCubic", "OutCubic", "InOutCubic", "OutInCubic",
		"InQuart", "OutQuart", "InOutQuart", "OutInQuart",
		"InQuint", "OutQuint", "InOutQuint", "OutInQuint",
		"InSine", "OutSine", "InOutSine", "OutInSine",
		"InExpo", "OutExpo", "InOutExpo", "OutInExpo",
		"InCirc", "OutCirc", "InOutCirc", "OutInCirc",
		"InBack", "OutBack", "InOutBack", "OutInBack"
	};

	return names;
}

EasingFunctions::EasingFunctions()
{
	m_func = &functions::easeLinear;
}

void EasingFunctions::setType(Type type)
{ 
	m_type = type;
	switch (type)
	{
	case Type::Linear:		m_func = &functions::easeLinear;		break;
	case Type::InQuad:		m_func = &functions::easeInQuad;		break;
	case Type::OutQuad:		m_func = &functions::easeOutQuad;		break;
	case Type::InOutQuad:	m_func = &functions::easeInOutQuad;		break;
	case Type::OutInQuad:	m_func = &functions::easeOutInQuad;		break;
	case Type::InCubic:		m_func = &functions::easeInCubic;		break;
	case Type::OutCubic:	m_func = &functions::easeOutCubic;		break;
	case Type::InOutCubic:	m_func = &functions::easeInOutCubic;	break;
	case Type::OutInCubic:	m_func = &functions::easeOutInCubic;	break;
	case Type::InQuart:		m_func = &functions::easeInQuart;		break;
	case Type::OutQuart:	m_func = &functions::easeOutQuart;		break;
	case Type::InOutQuart:	m_func = &functions::easeInOutQuart;	break;
	case Type::OutInQuart:	m_func = &functions::easeOutInQuart;	break;
	case Type::InQuint:		m_func = &functions::easeInQuint;		break;
	case Type::OutQuint:	m_func = &functions::easeOutQuint;		break;
	case Type::InOutQuint:	m_func = &functions::easeInOutQuint;	break;
	case Type::OutInQuint:	m_func = &functions::easeOutInQuint;	break;
	case Type::InSine:		m_func = &functions::easeInSine;		break;
	case Type::OutSine:		m_func = &functions::easeOutSine;		break;
	case Type::InOutSine:	m_func = &functions::easeInOutSine;		break;
	case Type::OutInSine:	m_func = &functions::easeOutInSine;		break;
	case Type::InExpo:		m_func = &functions::easeInExpo;		break;
	case Type::OutExpo:		m_func = &functions::easeOutExpo;		break;
	case Type::InOutExpo:	m_func = &functions::easeInOutExpo;		break;
	case Type::OutInExpo:	m_func = &functions::easeOutInExpo;		break;
	case Type::InCirc:		m_func = &functions::easeInCirc;		break;
	case Type::OutCirc:		m_func = &functions::easeOutCirc;		break;
	case Type::InOutCirc:	m_func = &functions::easeInOutCirc;		break;
	case Type::OutInCirc:	m_func = &functions::easeOutInCirc;		break;
	case Type::InBack:		m_func = &functions::easeInBack;		break;
	case Type::OutBack:		m_func = &functions::easeOutBack;		break;
	case Type::InOutBack:	m_func = &functions::easeInOutBack;		break;
	case Type::OutInBack:	m_func = &functions::easeOutInBack;		break;
	}
}

float EasingFunctions::valueForProgress(float progress) const
{
	return m_func(progress);
}

} // namespace helper

} // namespace panda
