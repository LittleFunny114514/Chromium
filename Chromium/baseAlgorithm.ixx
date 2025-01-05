module;
#include "base.macro.hpp"
export module Chromium.baseAlgorithm;
import Chromium.base;
import Chromium.NumT.base;
import Chromium.NumT.complex;
export namespace Chromium
{
	template <typename T>
	T qpow(T base, T pow, T mod)
	{
		T ret = 1; // base%=mod;
		while (pow != 0)
		{
			if (pow & 1)
			{
				ret %= mod;
				ret *= base;
			}
			base *= base;
			base %= mod;
			pow >>= 1;
		}
		ret %= mod;
		return ret;
	}
	/*
	 * 推荐原根和模：
	 * 5 % 0xC0000001
	 * 3 % 0x1180000001
	 * 7 % 0x1200000001
	 * 3 % 0x3A00000000000001
	 * % Int({ 1,0,12,0,0,0 }) //0xC0000000000000001
	 * % Int({ 1, 2147483648, 4, 0, 0, 0 }) //0x48000000000000001
	 * 原根表：
	 * https://www.cnblogs.com/LzyRapx/p/9109496.html
	 */
	template <concepts::Ints saveT, concepts::Ints calcT, template <typename> typename arrT>
	void NTT(arrT<saveT>& arr, bool inverse, calcT g, saveT mod)
	{
		assert(arr.size() == MathHelper::least2expGeqN(arr.size()));
		for (size_t i = 0, j = 0; i < arr.size(); i++)
		{
			if (i < j)
				std::swap(arr[i], arr[j]);
			for (size_t k = arr.size() >> 1; (j ^= k) < k; k >>= 1);
		}
		calcT ni = qpow<calcT>(arr.size(), mod - 2, mod);
		for (size_t lggap = 1; (1 << (lggap & 0x3f)) <= arr.size(); lggap++)
		{
			calcT omega = qpow<calcT>(g, (mod - 1) >> lggap, mod);
			for (size_t left = 0; left < arr.size(); left += (1ull << lggap))
			{
				calcT mlt = 1;
				for (size_t i = left; i < left + (1ull << (lggap - 1)); i++)
				{
					calcT te = arr[i], to = mlt * arr[i + (1ull << (lggap - 1))] % mod;
					arr[i] = (te + to) % mod;
					arr[i + (1ull << (lggap - 1))] = (te + mod - to) % mod;
					mlt = mlt * omega % mod;
				}
			}
		}
		if (inverse)
		{
			for (saveT& num : arr)num = calcT(num) * ni % mod;
			std::reverse(arr.begin() + 1, arr.end());
		}
	}
	template <concepts::Ints T>
	T findPrimitiveRoot(T p)
	{
		std::vector<T> factors;
		T t = p - 1;
		for (T i = 2; i * i < p - 1; i++)
		{
			if (t % i == 0)
			{
				factors.push_back(i);
				while (t % i == 0)t /= i;
			}
		}
		if (t > 1)
			factors.push_back(t);
		for (T g = 2; g <= p; g++)
		{
			bool is_primitiveRoot = true;
			for (const T& n : factors)
			{
				if (qpow(g, (p - 1) / n, p) == 1)
				{
					is_primitiveRoot = false;
					break;
				}
			}
			if (is_primitiveRoot)return g;
		}
		return -1;
	}
	template <typename T, template <typename> typename arrT>
	void FFT(arrT<complex<T>>& arr, bool inverse)
	{
		using consts::dpi;
		size_t size = arr.size();
		size_t n = size, m = 0;
		for (; n &= n - 1; m++)
			if (m == 2)
				std::terminate(); // FFT只能处理arr长度为2^n的情况
		for (size_t i = 0, j = 0; i < size; i++)
		{
			if (i < j)
				std::swap(arr[i], arr[j]);
			for (alias::ullong k = size >> 1; (j ^= k) < k; k >>= 1);
		}
		for (size_t gap = 2; gap <= size; gap <<= 1)
		{
			complex<T> omega(cos(2 * dpi / gap), sin(2 * dpi / gap));
			if (inverse)
				omega.imag *= -1;
			for (size_t j = 0; j < size; j += gap)
			{
				complex<T> mlt = 1, te, to;
				for (size_t k = j; k < j + (gap >> 1); k++)
				{
					te = arr[k];
					to = arr[k + (gap >> 1)] * mlt;
					mlt *= omega;
					arr[k] = te + to;
					arr[k + (gap >> 1)] = te - to;
				}
			}
		}
		if (inverse)
			for (complex<T>& n : arr)
				n /= size;
	}
	template <typename T, typename arrT>
	void FFT2D(arrT& arr, bool inverse)
	{
		for (decltype(arr[0])& row : arr)
			FFT<T>(row, inverse);
		std::vector<T> col;
		col.resize(arr.size());
		for (size_t j = 0; j < arr[0].size(); j++)
		{
			for (size_t i = 0; i < arr.size(); i++)
				col[i] = arr[i][j];
			FFT<T>(col, inverse);
			for (size_t i = 0; i < arr.size(); i++)
				arr[i][j] = col[i];
		}
	}
}