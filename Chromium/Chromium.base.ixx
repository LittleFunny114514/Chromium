module;
#include "base.macro.hpp"
#include <cstdlib>

export module Chromium.base;

export import std;
export import std.compat;
// export void Assert(bool expression) { assert(expression); }
export namespace Chromium
{
	namespace consts
	{
		const double dpi = 3.1415926535897932;
		const long double ldPi = static_cast<long double>(3.1415926535897932384626);
	}
	namespace alias
	{
		typedef unsigned long ulong;
		typedef unsigned long long ullong;
		typedef long long llong;
		typedef short hint;
		typedef unsigned short uhint;
		typedef unsigned char uchar;
	}
	using namespace alias;
	namespace config
	{
		const size_t MAX_ARR_CACHE_SIZE = 8;
	}
	namespace concepts
	{
		template<template<typename>typename TCon>
		concept Container = requires(TCon<int> con)
		{
			con.begin(), con.end();
		};
	}
	template<typename T>
	void fill(T* dst, size_t cnt, const T& val)
	{
		for (; cnt--; dst++)*dst = val;
	}
	namespace MathHelper {
		template<typename T>
		requires std::integral<T>|| std::is_floating_point<T>::value
		T clamp(T x, T min, T max)
		{
			if (x < min)return min;
			if (x > max)return max;
			return x;
		}
		template <std::integral T>
		T least2expGeqN(T n)
		{
			n -= 1;
			n |= n >> 1;
			n |= n >> 2;
			n |= n >> 4;
			if constexpr (sizeof(T) > 1)
				n |= n >> 8;
			if constexpr (sizeof(T) > 2)
				n |= n >> 16;
			if constexpr (sizeof(T) > 4)
				n |= n >> 32;
			return n + 1;
		}
		/* 获得一个整数类型的、向下取整的以2为底的对数值 */
		template <std::integral T>
		inline size_t log2i(T n)
		{
			// static_Assert(std::is_integral_v<T>, "Must be integer");
			Assert(n >= 0);
			T ret = 0;
			while (n >>= 1)
				ret++;
			return ret;
		}
		inline alias::ulong randUint()
		{
			ulong ret = 0;
#if RAND_MAX == 0x7fffffff
			ret = (::rand() << 1) + (::rand() & 1);
#elif RAND_MAX == 0x7fff
			ret = (::rand() << 17) + (::rand() << 2) + (::rand() & 3);
#else
			static const size_t n = log2i(RAND_MAX + 1) >> 3;
			for (size_t i = 0; i < n; i++)
			{
				ret <<= 8;
				ret += ::rand() & 0xFF;
			}
#endif
			return ret;
		}
		/* 相减两个数，如果是负数就返回0。通常用于unsigned类型 */
		template <std::integral T>
		inline T reduceMin0(T a, T b)
		{
			return (a > b) ? (a - b) : 0;
		}
	}


	/* 大于等于函数修正，防溢出 */
	
}