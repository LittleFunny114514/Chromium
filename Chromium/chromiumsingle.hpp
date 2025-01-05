#pragma once
#include<vector>
#include<cstdlib>
#include<complex>
#include<string>
#include<cmath>
#include<stdint.h>
#ifdef _MSVC_LANG
#undef min
#undef max
#define printf printf_s
#define scanf scanf_s
#define wprintf wprintf_s
#define wscanf wscanf_s
#define fprintf fprintf_s
#define fscanf fscanf_s
#define fwprintf fwprintf_s
#define fwscanf fwscanf_s
#define sprintf sprintf_s
#define sscanf sscanf_s
#define swprintf swprintf_s
#define swscanf swscanf_s
//template<typename T>inline T max(const T& a, const T& b) { return (a > b) ? a : b; }
//template<typename T>inline T min(const T& a, const T& b) { return (a < b) ? a : b; }
#endif //_MSVC_LANG
inline unsigned char log2i(uint64_t n){
	unsigned char ret = 0;
	while (n >>= 1)ret++;
	return ret;
}
inline uint32_t randgn(){
	uint32_t ret = 0;
#if RAND_MAX == 0x7fffffff
	ret = (::rand() << 1) + (::rand() & 1);
#elif RAND_MAX == 0x7fff
	ret = (::rand() << 17) + (::rand() << 2) + (::rand() & 3);
#else
	for (size_t i = 0; i < log2i(RAND_MAX + 1) >> 3; i++){
		ret <<= 8;
		ret += ::rand() & 0xFF;
	}
#endif
	return ret;
}
typedef std::complex<double> cp;
namespace Chromium {
	namespace consts {
		const size_t MAX_STRING_TO_NUM_BUFFEsize_t = 10;
		const bool POSITIVE = 0;
		const bool NEGATIVE = 1;
	}
	namespace alias {
		typedef unsigned long ulong;
		typedef unsigned long long ullong;
		typedef long long llong;
		typedef short hint;
		typedef unsigned short uhint;
		typedef std::vector<ulong> OrginInt;
		typedef std::vector<uhint> OrginInt16;
	}
	using namespace alias;
	template<typename T>
	T qpow(T base, T pow, T mod) {
		T ret = 1;
		while (pow != 0) {
			if (pow & 1) {
				ret %= mod;
				ret *= base;
			}
			base *= base % mod;
			base %= mod;
			pow >>= 1;
		}
		return ret;
	}
	template<typename T>
	void NTT(std::vector<T>& arr, bool inverse){
		;
	}
	void FFT(std::vector<std::complex<double>>& arr, bool inverse) {
		const double pi = 3.141592653589793;
		{
			size_t n = arr.size(), m = 0;
			for (; n &= n - 1; m++)if (m == 2)std::terminate(); //FFT只能处理arr长度为2^n的情况
		}
		for (size_t i = 0, j = 0; i < arr.size(); i++) {
			if (i < j)std::swap(arr[i], arr[j]);
			for (alias::ullong k = arr.size() >> 1; (j ^= k) < k; k >>= 1);
		}
		for (size_t gap = 2; gap <= arr.size(); gap <<= 1) {
			std::complex<double> omega(cos(2 * pi / gap), sin(2 * pi / gap));
			if (inverse)omega.imag(omega.imag() * -1);
			for (size_t j = 0; j < arr.size(); j += gap) {
				std::complex<double>mlt = 1, te, to;
				for (size_t k = j; k < j + (gap >> 1); k++) {
					te = arr[k]; to = arr[k + (gap >> 1)] * mlt;
					mlt *= omega;
					arr[k] = te + to;
					arr[k + (gap >> 1)] = te - to;
				}
			}
		}
	}
	class Int {
	protected:
		alias::OrginInt num;
	public:
		Int() {}
		Int(const alias::OrginInt& _num) { num = _num; }
		Int(alias::ulong n) { num = { n }; }
		Int(alias::ullong  n) { num = { alias::ulong(n >> 32),(alias::ulong)n }; }
		//Int(const std::string& str, size_t max_len) {
		//	
		//}
		std::string bin() {
			using alias::ulong;
			std::string str; str.resize(num.size() * 33);
			size_t idx = 0;
			for (size_t i = 0; i < num.size(); i++) {
				ulong t = num[i];
				for (size_t j = 0; j < 32; j++) {
					str[idx] = '0' + ((t & 0x80000000) >> 31);
					idx++;
					t <<= 1;
				}str[idx++] = ' ';
			}return str;
		}
		static Int rand(size_t size){
			alias::OrginInt ret;
			for (alias::ulong& n : ret) 
				n = randgn();
			return ret;
		}
		inline size_t size()const { return num.size(); }
		virtual void resize(size_t size) {
			using alias::llong;
			using alias::OrginInt;
			using std::max;
			using std::min;
			OrginInt buf = num;
			num.resize(size);
			if (buf.size()) {
				memcpy(&num[max(llong(num.size()) - llong(buf.size()), (llong)0)],
					&buf[max(llong(buf.size()) - llong(num.size()), (llong)0)],
					min(buf.size(), num.size()) * 4);
				if (bool(buf[0] & 0x80000000) && num.size() > buf.size()) {
					for (size_t i = 0; i < num.size() - buf.size(); i++)num[i] = 0xFFFFFFFF;
				}
			}
		}
		inline alias::ulong operator[](size_t idx)const {
			return ((num.size() - idx - 1) & 0x8000000000000000) ?
				((num[0] & 0x80000000) ? 0xFFFFFFFF : 0) :
				num[num.size() - idx - 1];
		}
		inline alias::ulong& operator[](size_t idx) { return static_cast<alias::ulong&>(num.at(num.size() - idx - 1)); }
		Int operator~()const {
			alias::OrginInt out = num;
			for (alias::ulong& n : out)n = ~n;
			return Int{ out };
		}
		Int operator<<(size_t offset)const {
			Int buf = *this;
			buf <<= offset;
			return buf;
		}
		Int operator>>(size_t offset)const {
			Int buf = *this;
			buf <<= offset;
			return buf;
		}
		Int& operator<<=(size_t offset) {
			for (size_t i = offset >> 5; i < num.size() - 1; i++) {
				num[i - (offset >> 5)] = num[i] << (offset & 0x1F);
				num[i - (offset >> 5)] += (alias::ullong)num[i + 1] << (offset & 0x1F) >> 32;
				//std::cout << this->bin() << std::endl;
			}num[num.size() - 1 - (offset >> 5)] = num[num.size() - 1] << offset;
			for (size_t i = 0; i < (offset >> 5); i++)num[num.size() - i - 1] = 0;
			return *this;
		}
		Int& operator>>=(size_t offset) {
			for (size_t i = num.size() - 1 - (offset >> 5); i > 0; i--) {
				num[i + (offset >> 5)] = num[i] >> (offset & 0x1F);
				num[i + (offset >> 5)] += (alias::ullong)num[i - 1] << 32 >> (offset & 0x1F);
			}num[offset >> 5] = num[0] >> offset;
			for (size_t i = 0; i < (offset >> 5); i++)num[i] = 0;
			return *this;
		}
		Int& operator+=(const Int& other) {
			using alias::ullong;
			using alias::ulong;
			ullong adbuf = 0;
			ulong last = 0;
			
			resize(std::max(num.size(), other.num.size()));
			for (size_t i = 0; i < num.size(); i++) {
				adbuf = (ullong)num[num.size() - i - 1] + other[i] + (ullong)last;
				num[num.size() - i - 1] = (ulong)adbuf;
				last = adbuf >> 32;
			}return *this;
		}
		Int& operator-=(const Int& other) {
			Int buf = other;
			buf = ~buf;
			buf += Int(1ul);
			(*this) += buf;
			return *this;
		}
		Int operator+(const Int& other)const {
			Int out = *this;
			out += other;
			return out;
		}
		Int operator-(const Int& other)const {
			Int out = *this;
			out -= other;
			return out;
		}
		/*Int operator*(const Int& other) {
			Int buf;
			buf.resize(max(num.size(), other.size()));
			for (size_t x = 0; x < num.size(), x++) {
				for (size_t y = 0; y < other.size(), x++) {

				}
			}
		}*/
	};
}