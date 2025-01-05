module;
#include "../base.macro.hpp"
export module Chromium.NumT.complex;
import Chromium.NumT.base;
import Chromium.mathbase;
export namespace Chromium {

	template<concepts::Reals T>
	class complex : concepts::TagT(CR_complex) {
		const static T pi = mathc::Pi<T>::val;
	public:
		T real = 0, imag = 0;
		complex(T Real = 0, T Imag = 0) :real(Real), imag(Imag) {}
		inline complex& operator=(const complex<T>&other) { real = other.real, imag = other.imag; return *this; }
		inline bool operator==(const complex<T>&other)const { return real == other.real && imag = other.imag; }
		inline bool operator!=(const complex<T>&other)const { return !operator==(other); }
		inline complex operator+(const complex<T>&other)const { return { real + other.real,imag + other.imag }; }
		inline complex& operator+=(const complex<T>&other) { *this = *this + other; return *this; }
		inline complex operator-(const complex<T>&other)const { return { real - other.real,imag - other.imag }; }
		inline complex& operator-=(const complex<T>&other) { *this = *this - other; return *this; }
		inline complex operator*(const complex<T>&other)const {
			return { real * other.real - imag * other.imag,
					real * other.imag + imag * other.real };
		}
		inline complex& operator*=(const complex<T>&other) { *this = *this * other; return *this; }
		inline complex operator/(const complex<T>&other)const {
			T t = other.real * other.real + other.imag * other.imag;
			return operator*({ other.real / t,other.imag * -1 / t });
		}
		inline complex& operator/=(const complex<T>&other) { *this = *this / other; return *this; }
		/* 注意这个类的^表示指数 */
		inline complex operator^(const complex<T>&other) {
			T theta = atan(imag / real) + other.imag;
			complex mod = mod(*this) * exp(other.real);
			return { mod * cos(theta),mod * sin(theta) };
		}
		inline T arg() { return real == 0 ? (imag > 0 ? complex<T>::pi / 2 : complex<T>::pi * 3 / 2) : atan(imag / real); };
	};
	template<typename T>
	inline T mod(const complex<T>& z) { return sqrt(z.real * z.real + z.imag * z.imag); }
	template<typename T>
	complex<T> log(complex<T>& z) { return { log(mod(z)), z.arg()}; }
}