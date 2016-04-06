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


class PANDA_CORE_API IntVector
{
public:
	std::vector<int> values;

	IntVector() = default;
	IntVector(const std::vector<int>& v);

	IntVector& operator=(const std::vector<int>& v);
	IntVector& operator=(const IntVector& v);

	// Operation on all values
	IntVector operator+(const int& v) const;
	IntVector operator-(const int& v) const;
	IntVector& operator+=(const int& v);
	IntVector& operator-=(const int& v);

	IntVector operator*(int v) const;
	IntVector operator/(int v) const;
	IntVector& operator*=(int v);
	IntVector& operator/=(int v);

	friend IntVector operator*(int v, const IntVector& p);

	IntVector reversed() const;
	void reverse();

	bool operator==(const IntVector& p) const;
	bool operator!=(const IntVector& p) const;
};

#ifndef PANDA_BUILD_CORE
extern template class PANDA_CORE_API Data<IntVector>;
extern template class PANDA_CORE_API Data<std::vector<IntVector>>;
#endif

//****************************************************************************//

inline IntVector::IntVector(const std::vector<int>& v)
	: values(v) {}

inline IntVector& IntVector::operator=(const std::vector<int> &v)
{ values = v; return *this; }

inline IntVector& IntVector::operator=(const IntVector &v)
{ values = v.values; return *this; }

inline IntVector& IntVector::operator+=(const int& v)
{ for (auto& i : values) i += v; return *this; }

inline IntVector& IntVector::operator-=(const int& v)
{ for (auto& i : values) i -= v; return *this; }

inline IntVector& IntVector::operator*=(int v)
{ for (auto& i : values) i *= v; return *this; }

inline IntVector& IntVector::operator/=(int v)
{ for (auto& i : values) i /= v; return *this; }

inline bool IntVector::operator==(const IntVector& p) const
{ return values == p.values; }

inline bool IntVector::operator!=(const IntVector& p) const
{ return values != p.values; }

} // namespace types

} // namespace panda
