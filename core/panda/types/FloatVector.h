#pragma once

#include <panda/core.h>

#ifndef PANDA_BUILD_CORE
#include <panda/data/Data.h>
#endif

#include <vector>

namespace panda
{

namespace types
{


class PANDA_CORE_API FloatVector
{
public:
	std::vector<float> values;

	FloatVector() = default;
	FloatVector(const std::vector<float>& v);

	FloatVector& operator=(const std::vector<float>& v);
	FloatVector& operator=(const FloatVector& v);

	// Operation on all values
	FloatVector operator+(const float& v) const;
	FloatVector operator-(const float& v) const;
	FloatVector& operator+=(const float& v);
	FloatVector& operator-=(const float& v);

	FloatVector operator*(float v) const;
	FloatVector operator/(float v) const;
	FloatVector& operator*=(float v);
	FloatVector& operator/=(float v);

	friend FloatVector operator*(float v, const FloatVector& p);

	FloatVector reversed() const;
	void reverse();

	bool operator==(const FloatVector& p) const;
	bool operator!=(const FloatVector& p) const;
};

#ifndef PANDA_BUILD_CORE
extern template class PANDA_CORE_API Data<FloatVector>;
extern template class PANDA_CORE_API Data<std::vector<FloatVector>>;
#endif

//****************************************************************************//

inline FloatVector::FloatVector(const std::vector<float>& v)
	: values(v) {}

inline FloatVector& FloatVector::operator=(const std::vector<float> &v)
{ values = v; return *this; }

inline FloatVector& FloatVector::operator=(const FloatVector &v)
{ values = v.values; return *this; }

inline FloatVector& FloatVector::operator+=(const float& v)
{ for (auto& i : values) i += v; return *this; }

inline FloatVector& FloatVector::operator-=(const float& v)
{ for (auto& i : values) i -= v; return *this; }

inline FloatVector& FloatVector::operator*=(float v)
{ for (auto& i : values) i *= v; return *this; }

inline FloatVector& FloatVector::operator/=(float v)
{ for (auto& i : values) i /= v; return *this; }

inline bool FloatVector::operator==(const FloatVector& p) const
{ return values == p.values; }

inline bool FloatVector::operator!=(const FloatVector& p) const
{ return values != p.values; }

} // namespace types

} // namespace panda
