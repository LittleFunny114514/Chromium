module;
#include "../base.macro.hpp"
#include "NumT.macro.hpp"
export module Chromiun.NumT.Integer;
import Chromium.base;
import Chromium.utilities;
import Chromium.baseAlgorithm;
import Chromium.NumT.base;
export namespace Chromium 
{
	using namespace alias;
	template <bool Signed, bool dyn>
	class Int : concepts::TagT(hiInt)
	{
		using datT = config::hInt_datT;
		/* 我找不到更好的写法了 */
		friend class Int<false, false>;
		friend class Int<true, false>;
		friend class Int<false, true>;
		friend class Int<true, true>;

	protected:
		datT num = datT(config::MIN_SIZE_OF_INT);

	public:
		/* 初始化 */
		Int() {}
		/* 小端！小端！小端！ */
		Int(const datT & _num)
		{
			num = _num;
			if (size() < config::MIN_SIZE_OF_INT)
				resize(config::MIN_SIZE_OF_INT);
		}
		template <std::integral T>
		Int(T n) { *this = n; }
		template <concepts::hiInts IntT>
		Int(const IntT & other) { num = other.num; }
		//~Int() { num.clear(); }

		/* 赋值，转化 */
		template <concepts::hiInts IntT>
		inline Int& operator=(const IntT & other)
		{
			size_t oldsize = size();
			num = other.num;
			if (!dyn)
				resize(oldsize);
			return *this;
		}
		template <std::integral T>
		Int& operator=(T low)
		{
			static const size_t minsize = config::MIN_SIZE_OF_INT;
			using namespace alias;
			if (dyn)
				num.resize(minsize);
			num[0] = (ullong)low;
			if constexpr (sizeof(T) > sizeof(Int_cellT))
				num[1] = ullong(low >> 32);
			ullong place = (low < 0) ? 0xFFFFFFFF : 0;
			for (size_t i = sizeof(T) >> 2; i < size(); i++)
				num[i] = place;
			return *this;
		}
		template <std::integral T>
		operator T() const { return ullong(num[1]) << 32 | num[0]; }

		/* 输出 */
		template<bool gap = true>
		std::string bin() const
		{
			using alias::Int_cellT;
			std::string str;
			str.resize(num.size() * (8 * sizeof(Int_cellT) + gap));
			size_t idx = 0;
			for (size_t i = num.size(); i--;)
			{
				ulong t = num[i];
				for (size_t j = 0; j < 8 * sizeof(Int_cellT); j++)
				{
					str[idx] = '0' + (t >> (8 * sizeof(Int_cellT) - 1));
					idx++;
					t <<= 1;
				}
				str[idx++] = ' ';
			}
			return str;
		}
		template<bool gap = true>
		std::string hex() const
		{
			std::ostringstream ss;
			ss << std::setiosflags(std::ios::uppercase) << std::hex;
			for (size_t i = num.size(); i--;)
			{
				ss << std::setw(8) << std::setfill('0') << num[i];
				if constexpr (gap)ss << ' ';
			}
			return ss.str();
		}

		static Int rand(size_t size)
		{
			datT ret(size);
			for (Int_cellT& n : ret)
				n = MathHelper::randUint();
			return ret;
		}

		/* 分块、分位有关的 */
		Int& setBit(size_t idx, bool state)
		{
			static const size_t sizecell = sizeof(Int_cellT) << 3, lgcellbit = MathHelper::log2i(sizecell);
			if (state)
				num[idx >> lgcellbit] |= 1 << (idx & (sizecell - 1));
			else
				num[idx >> lgcellbit] &= ~(1 << (idx & (sizecell - 1)));
			return *this;
		}
		size_t usedBlockCnt() const
		{
			size_t i = size();
			while (i-- != 0 && ((num[i] == 0xFFFFFFFF && Signed) || !num[i]));
			if (getSign() && (int)num[i] >= 0 && i + 1 < size())i++;
			return i + 1;
		}

		inline Int_cellT operator[](size_t idx) const
		{
			return (idx >= num.size()) ? (getSign() ? 0xFFFFFFFF : 0) : num[idx];
		}
		inline Int_cellT& operator[](size_t idx) { return num.at(idx); }

		inline bool getSign() const { return Signed && (num[size() - 1] & Int_cellT(-1)); }
		inline size_t size() const { return num.size(); }
		virtual Int& resize(size_t newsize)
		{
			using alias::llong;
			using std::max, std::min;
			newsize = max(newsize, config::MIN_SIZE_OF_INT);
			if (newsize == size())
				return *this;

			size_t cellbit = sizeof(Int_cellT) << 3;
			if (size() && newsize) // 如果原来大小不是0，才复制数据
			{
				datT buf = num;
				num.resize(newsize, (Signed && (buf[buf.size() - 1] >> (cellbit - 1))) ? -1 : 0);
				memcpy(&num[0], &buf[0],
					min(buf.size(), num.size()) * sizeof(Int_cellT));
			}
			else
				num.resize(newsize);
			return *this;
		}
		inline Int& setLenForce(size_t size) { return num.clear(), num.resize(size, 0), * this; }

		/* 位运算 */
		Int operator~() const
		{
			Int out = *this;
			for (size_t i = 0; i < out.size(); i++)
				out[i] = ~out[i];
			return out;
		}
		template <concepts::hiInts IntT>
		Int& operator&=(const IntT & other)
		{
			for (size_t i = 0; i < size(); i++)
				num[i] &= other[i];
			return *this;
		}
		template <concepts::hiInts IntT>
		Int& operator|=(const IntT & other)
		{
			for (size_t i = 0; i < size(); i++)
				num[i] |= other[i];
			return *this;
		}
		template <concepts::hiInts IntT>
		Int& operator^=(const IntT & other)
		{
			for (size_t i = 0; i < size(); i++)
				num[i] ^= other[i];
			return *this;
		}

		Int& operator>>=(size_t offset)
		{
			using cellT = Int_cellT;
			using calcT = Int_calcT;
			static const size_t dig = sizeof(Int_cellT) * 8,
				ddig = MathHelper::log2i(dig);
			cellT place = getSign() ? 0xFFFFFFFF : 0;
			for (size_t i = offset >> ddig; i < num.size() - 1; i++) // 移动大部分的数据
			{
				num[i - (offset >> ddig)] = num[i] >> (offset & (dig - 1));
				num[i - (offset >> ddig)] += (calcT)num[i + 1] << dig >> (offset & (dig - 1));
			}
			num[size() - 1 - (offset >> ddig)] = num[size() - 1] >> (offset & (dig - 1));
			for (size_t i = 0; i < (offset >> ddig); i++)
				num[size() - i - 1] = place;
			return *this;
		}
		Int& operator<<=(size_t offset)
		{
			static const size_t dig = sizeof(Int_cellT) * 8,
				ddig = MathHelper::log2i(dig);
			for (size_t i = num.size() - 1 - (offset >> ddig); i > 0 && i < num.size(); i--)
			{
				num[i + (offset >> ddig)] = num[i] << (offset & (dig - 1));
				num[i + (offset >> ddig)] += (Int_calcT)num[i - 1] << (offset & (dig - 1)) >> dig;
			}
			num[offset >> ddig] = num[0] << (offset & (dig - 1));
			for (size_t i = 0; i < (offset >> ddig); i++)
				num[i] = 0;
			return *this;
		}
		mkOprWithConstruct(Int, |=, std::integral);
		mkOprWithConstruct(Int, &=, std::integral);
		mkOprWithConstruct(Int, ^=, std::integral);
		mkOprWithOprAndAssign(Int, | , concepts::Ints);
		mkOprWithOprAndAssign(Int, &, concepts::Ints);
		mkOprWithOprAndAssign(Int, ^, concepts::Ints);
		mkOprWithOprAndAssign(Int, << , std::integral);
		mkOprWithOprAndAssign(Int, >> , std::integral);

		/* 比较 */
		template <concepts::hiInts IntT>
		bool operator==(const IntT & other) const
		{
			if (num.size() == other.size())
				return num == other.num;
			for (size_t i = 0; i < std::max(num.size(), other.size()); i++)
				if (operator[](i) != other[i])
					return false;
			return true;
		}
		template <concepts::hiInts IntT>
		inline bool operator!=(const IntT & other) const { return !(*this == other); }
		template <concepts::hiInts IntT>
		bool operator>(const IntT & other) const
		{
			using std::max;
			using std::min;
			bool t;
			size_t Min = min(size(), other.size()), Max = max(size(), other.size());
			t = signed(operator[](Max - 1)) > signed(other[Max - 1]); // operator[]()const自带补充前导0
			if (Signed && signed(operator[](Max - 1)) != signed(other[Max - 1]))
				return t;						  // 有符号类型直接比最高位块（带符号那块）
			for (size_t i = Signed; i < Max; i++) // 后面当无符号的比
			{
				t = operator[](Max - i - 1) > other[Max - i - 1];
				if (operator[](Max - i - 1) != other[Max - i - 1])
					return t;
			}
			return false;
		}
		template <concepts::hiInts IntT>
		inline bool operator>=(const IntT & other) const { return !(other > *this); }
		template <concepts::hiInts IntT>
		inline bool operator<(const IntT & other) const { return !(*this >= other); }
		template <concepts::hiInts IntT>
		inline bool operator<=(const IntT & other) const { return !(*this > other); }
		mkOprWithConstructConst(Int, == , std::integral);
		mkOprWithConstructConst(Int, != , std::integral);
		mkOprWithConstructConst(Int, > , std::integral);
		mkOprWithConstructConst(Int, < , std::integral);
		mkOprWithConstructConst(Int, >= , std::integral);
		mkOprWithConstructConst(Int, <= , std::integral);

		/* 加减 */
		template <concepts::hiInts IntT>
		Int& operator+=(const IntT & other)
		{
			using alias::Int_calcT;
			using alias::Int_cellT;
			static const size_t cellbit = sizeof(Int_cellT) * 8;
			Int_calcT adbuf = 0;
			if (dyn && other.size() > size())
				resize(other.size());
			size_t usdblkcntr = other.usedBlockCnt(), usdblkcnt;
			for (size_t i = 0; i < size() && (adbuf || i < usdblkcntr || other[i]); i++)
			{
				adbuf += (Int_calcT)num[i] + other[i];
				num[i] = (Int_cellT)adbuf;
				adbuf >>= cellbit;
			}
			if (dyn && (usdblkcnt = usedBlockCnt()) != num.size() - 1)num.resize(usdblkcnt + 1);
			return *this;
		}
		template <concepts::hiInts IntT>
		Int& operator-=(const IntT & other)
		{
			(*this) += -other;
			return *this;
		}
		inline Int operator-() const { return (~*this) += 1; }
		mkOprWithOprAndAssign(Int, +, concepts::Ints);
		mkOprWithOprAndAssign(Int, -, concepts::Ints);
		mkOprWithConstruct(Int, +=, std::integral);
		mkOprWithConstruct(Int, -=, std::integral);

		template <std::integral T>
		Int operator*(T low) const
		{
			Int ret;
			ret.resize(size());
			static const size_t cellbit = sizeof(Int_cellT) * 8;
			Int_calcT t = 0;
			for (size_t i = 0; i < num.size(); i++)
			{
				t += (Int_calcT)ret[i] + (Int_calcT)num[i] * (Int_cellT)low
					+ (i ? Int_calcT(num[i - 1]) * (low >> cellbit) : 0);
				ret[i] = t;
				t >>= cellbit;
			}
			return ret;
		}
		template <concepts::hiInts IntT>
		Int operator*(const IntT & other) const
		{
			using namespace alias;
			using std::min, std::max, std::vector;
			static const size_t cellbit = sizeof(Int_cellT) * 8;
			size_t usdblkcntl = usedBlockCnt(), usdblkcntr = other.usedBlockCnt();
			datT ret(dyn ? usdblkcntl + usdblkcntr + 1 : size());
			if (std::min(num.size(), other.size()) <= config::MAX_SIZE_PRIMARY_PROCESS)
			{
				size_t X = usdblkcntl, Y = usdblkcntr;

				for (size_t x = 0; x < X; x++)
				{
					for (size_t y = 0; y < Y && x + y < ret.size(); y++)
					{
						Int_calcT t = (Int_calcT)num[x] * other.num[y];
						for (size_t i = x + y; t && i < ret.size(); i++)
						{
							t += ret[i];
							ret[i] = t;
							t >>= cellbit;
						}
					}
				}
				size_t i = min(X + Y, ret.size() - 1);
				if (Signed && (int)ret[i] < 0)
					while (++i < ret.size())
						ret[i] = (Int_cellT)-1;
				return ret;
			}
			size_t alen = usdblkcntl + 1, blen = usdblkcntr + 1, arrlen = MathHelper::least2expGeqN(alen + blen);

			if (arrlen <= 0x3000) {
				alen <<= 2, blen <<= 2, arrlen <<= 2;
				ullong g = 5, mod = 3221225473;
				vector<ulong>a(arrlen), b(arrlen);
				for (size_t i = 0; (i << 2) < alen; i++)
				{
					ulong t = operator[](i);
					a[(i << 2)] = (uchar)t;
					a[(i << 2) | 1] = (uchar)(t >> 8);
					a[(i << 2) | 2] = (uchar)(t >> 16);
					a[(i << 2) | 3] = (uchar)(t >> 24);
				}
				for (size_t i = 0; (i << 2) < blen; i++)
				{
					ulong t = other[i];
					b[(i << 2)] = (uchar)t;
					b[(i << 2) | 1] = (uchar)(t >> 8);
					b[(i << 2) | 2] = (uchar)(t >> 16);
					b[(i << 2) | 3] = (uchar)(t >> 24);
				}
				NTT<ulong, ullong, vector>(a, false, g, mod);
				NTT<ulong, ullong, vector>(b, false, g, mod);
				for (size_t i = 0; i < arrlen; i++)
					a[i] = ullong(a[i]) * b[i] % mod;
				NTT<ulong, ullong, vector>(a, true, g, mod);
				for (size_t i = 0; i + 1 < arrlen; i++)
					a[i + 1] += a[i] >> 8, a[i] = (uchar)a[i];
				size_t i = 0;
				for (; i < ret.size() && (i << 2) < alen + blen; i++)
					ret[i] = (a[(i << 2) | 3] << 24) | (a[(i << 2) | 2] << 16) | (a[(i << 2) | 1] << 8) | a[i << 2];
				if (Signed && ret[(alen + blen >> 2) - 1] & 0x80000000)
					while (i < ret.size())ret[i++] = 0xFFFFFFFF;
				return ret;
			}
			alen <<= 1, blen <<= 1, arrlen <<= 1;

			Int<false, false> g = 3;
			Int<false, false> mod = 4179340454199820289;
			vector<Int_calcT> a(arrlen), b(arrlen);

			for (size_t i = 0; i < alen; i += 2)
				a[i] = (uhint) operator[](i >> 1), a[i | 1] = uhint(operator[](i >> 1) >> 16);
			for (size_t i = 0; i < blen; i += 2)
				b[i] = (uhint)other[i >> 1], b[i | 1] = uhint(other[i >> 1] >> 16);

			NTT<Int_calcT, Int<false, false>, vector>(a, false, g, mod);
			NTT<Int_calcT, Int<false, false>, vector>(b, false, g, mod);

			for (size_t i = 0; i < arrlen; i++)
				a[i] = Int_calcT(Int<false, false>(a[i]) * b[i] % mod);

			NTT<Int_calcT, Int<false, false>, vector>(a, true, g, mod);

			for (size_t i = 0; i + 1 < arrlen; i++)
				a[i + 1] += a[i] >> 16, a[i] = (uhint)a[i];
			size_t i = 0;
			for (; i < ret.size() && (i << 1) < alen + blen; i++)
				ret[i] = (a[(i << 1) | 1] << 16) | a[i << 1];
			if (Signed && ret[(alen + blen >> 1) - 1] & 0x80000000)
				while (i < ret.size())ret[i++] = 0xFFFFFFFF;
			return ret;
		}
		mkOprAndAssignWithOpr(Int, *, concepts::Ints);
		/* 最基本的除法，时间复杂度:O(size()*other.size()) */
		void divStandard(const Int & other, Int & quotient, Int & rem) const
		{
			using namespace std;
			Assert(other != 0);
			rem = *this;
			if (operator<(other))
				return;
			static const size_t cellbit = sizeof(Int_cellT) << 3, lgcellbit = MathHelper::log2i(cellbit);
			size_t usdblkcntl = usedBlockCnt(), usdblkcntr = other.usedBlockCnt();
			quotient.setLenForce(dyn ? MathHelper::reduceMin0(usdblkcntl, usdblkcntr) : size());
			size_t i = std::min(quotient.size() * cellbit, (usdblkcntl - usdblkcntr + 1) << lgcellbit);
			Int sub = other;
			sub.resize(max(sub.size(), usdblkcntl + usdblkcntr)) <<= i;
			for (; i != SIZE_MAX; i--)
			{
				if (rem >= sub)
				{
					rem -= sub;
					quotient.setBit(i, 1);
				}
				sub >>= 1;
			}
		}

		template <concepts::hiInts IntT>
		inline Int operator/(const IntT & other) const
		{
			Int q, r;
			divStandard(other, q, r);
			return q;
		}
		template <concepts::hiInts IntT>
		inline Int operator%(const IntT & other) const
		{
			Int q, r;
			divStandard(other, q, r);
			return r;
		}
		mkOprWithConstruct(Int, / , std::integral);
		mkOprAndAssignWithOpr(Int, / , concepts::Ints);
		mkOprWithConstruct(Int, %, std::integral);
		mkOprAndAssignWithOpr(Int, %, concepts::Ints);
	};
	template <concepts::hiInts IntT>
	size_t log2i(const IntT& num)
	{
		size_t t = num.usedBlockCnt() - 1;
		return t * sizeof(config::Int_cellT) * 8 + log2i(num[t]);
	}
}