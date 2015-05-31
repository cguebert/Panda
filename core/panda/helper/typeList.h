#ifndef HELPER_TYPELIST_H
#define HELPER_TYPELIST_H

#include <tuple>

namespace panda
{

namespace helper
{

namespace details
{
	template<int I, int N, class F, class... T> struct ForEachTypeImpl
	{
		void operator()(F f) const
		{
			using Tp = typename std::tuple_element<I, std::tuple<T...>>::type;

			Tp val{};
			f(val);
			ForEachTypeImpl< I + 1, N, F, T... >()(f);
		}
	};

	template<int N, class F, class... T> struct ForEachTypeImpl < N, N, F, T... >
	{
		void operator()(F) const { }
	};

	template <class L, class F> struct ForEachType
	{
		void operator()(F) { }
	};

	template <template<class...> class L, class F, class...T>
	struct ForEachType < L<T...>, F >
	{
		void operator()(F f)
		{
			ForEachTypeImpl<0, sizeof...(T), F, T...>()(f);
		}
	};

	template<class L, class... T> struct PushFrontImpl;

	template<template<class...> class L, class... U, class... T>
	struct PushFrontImpl<L<U...>, T...>
	{
		using type = L<T..., U...>;
	};

	template<class L, class... T> struct PushBackImpl;

	template<template<class...> class L, class... U, class... T>
	struct PushBackImpl<L<U...>, T...>
	{
		using type = L<U..., T...>;
	};

} // namespace details

/// Add one or multiple types to the front of a type list
template<class L, class... T>
using push_front = typename details::PushFrontImpl<L, T...>::type;

/// Add one or multiple types to the back of a type list
template<class L, class... T>
using push_back = typename details::PushBackImpl<L, T...>::type;

/// Call a function for each type in a type list
template <class L, class F>
void for_each_type(F f)
{
	details::ForEachType<L, F>()(f);
}

} // namespace helper

} // namespace panda

#endif // HELPER_TYPELIST_H
