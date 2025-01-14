module;
#include "../base.macro.hpp"
#include "../NumT/NumT.macro.hpp"
export module Chromium.NN.LA;
import Chromium.base;
export import Chromium.mathbase;
export import Chromium.utilities;
using Chromium::concepts::Reals, Chromium::pos2;
export namespace Chromium
{
	namespace config
	{
		const size_t MM_BLOCK_SIZE = 64;
	}
	template<Reals T>
	class Vector
	{
	public:
		array<T> dat;
		Vector() {}
		Vector(size_t dim, T fill = T()) { dat.resize(dim, fill); }
		Vector& operator+=(const Vector& other)
		{
			Assert(dat.size() == other.dat.size());
			for (size_t i = 0; i < dat.size(); i++)
				dat[i] += other.dat[i];
			return *this;
		}
		Vector& operator-=(const Vector& other)
		{
			Assert(dat.size() == other.dat.size());
			for (size_t i = 0; i < dat.size(); i++)
				dat[i] -= other.dat[i];
			return *this;
		}
		Vector operator-()
		{
			Vector ret = *this;
			for (T& v : ret.dat)v = -v;
			return ret;
		}
		mkOprWithOprAndAssign(Vector, +, Reals);
		mkOprWithOprAndAssign(Vector, -, Reals);
		// dot product
		T operator*(const Vector& other)
		{
			Assert(dat.size() == other.dat.size());
			size_t i = 0; T ret = T();
			for (size_t i = 0; i < dat.size(); i++)
				ret += dat[i] * other.dat[i];
			return *this;
		}
	};
	template<concepts::Reals T>
	class Matrix
	{
	public:
		array2d<T>dat;
		Matrix() {}
		Matrix(size_t w, size_t h, T fill = T()) { dat.resize(w, h, fill); }
		Matrix(size_t sidelen) { dat.resize(sidelen, sidelen, T()); }
		Matrix& operator+=(const Matrix& other)
		{
			Assert(dat.width() == other.dat.width() && dat.height() == other.dat.height());
			T* it = dat.begin(), * ito = other.dat.begin(), * end = dat.end();
			while (it != end) *(it++) += *(ito++);
			return *this;
		}
		Matrix& operator-=(const Matrix& other)
		{
			Assert(dat.width() == other.dat.width() && dat.height() == other.dat.height());
			T* it = dat.begin(), * ito = other.dat.begin(), * end = dat.end();
			while (it != end) *(it++) -= *(ito++);
			return *this;
		}
		Matrix& operator-()
		{
			Matrix ret = *this;
			T* it = dat.begin(), * end = dat.end();
			while (it != end) *it = -*it, it++;
			return *this;
		}
		mkOprWithOprAndAssign(Matrix, +, Reals);
		mkOprWithOprAndAssign(Matrix, -, Reals);
		Matrix operator*(const Matrix&);

	};
}
using Chromium::Matrix;
namespace inner {
	/*
	template<Reals T>
	void MM_naive(const Matrix<T>& A, const Matrix<T>& B, Matrix<T>& C)
	{
		size_t w = C.dat.width(), h = C.dat.height(), n = A.dat.width();
		T* itc = C.dat.begin();
		for (size_t i = 0; i < w; i++)
		{
			const T* Bbegin = B.dat[i];
			for (size_t j = 0; j < h; j++)
			{
				const T* Abegin = A.dat.begin() + j, * ita = Abegin, * itb = Bbegin;
				for (size_t k = 0; k < n; k++)
					*itc += *ita * *(itb++), ita += h;
				itc++;
			}
		}
	}*/
	template<Reals T>
	void MM_naive(const Matrix<T>& A, const Matrix<T>& B, Matrix<T>& C)
	{
		using namespace Chromium::alias;
		size_t w = C.dat.width(), h = C.dat.height(), n = A.dat.width();
		const T* itb = B.dat.begin(), * ita;
		T* pivc = C.dat.begin(), * itc;
		for (llong i = 0; i < w; i++)
		{
			ita = A.dat.begin();
			for (size_t j = 0; j < n; j++)
			{
				itc = pivc;
				for (size_t k = 0; k < h; k++)
					*(itc++) += *(ita++) * *itb;
				itb++;
			}
			pivc = itc;
		}
		
	}
	template<std::integral T>
	T divceil(T a, T b) { return a / b + bool(a % b); }
	template<Reals T>
	void MM_block(const Matrix<T>& A, const Matrix<T>& B, Matrix<T>& C)
	{
		using namespace Chromium::alias;
		using Chromium::config::MM_BLOCK_SIZE;
		using std::max, std::min;
		size_t w = C.dat.width(), h = C.dat.height(), n = A.dat.width();
		size_t W = divceil(w, MM_BLOCK_SIZE), H = divceil(h, MM_BLOCK_SIZE), N = divceil(n, MM_BLOCK_SIZE);
		Matrix<T> a(MM_BLOCK_SIZE), b(MM_BLOCK_SIZE), c(MM_BLOCK_SIZE);
		size_t i = 0;
//#pragma omp parallel for
		for (llong I = 0; I < W; I++)
		{
			for (size_t J = 0, j = 0; J < H; J++, j += MM_BLOCK_SIZE)
			{
				for (T& num : c.dat)num = 0;
				// width and height of blocks of product
				size_t ml = min(MM_BLOCK_SIZE, w - i), nl = min(MM_BLOCK_SIZE, h - j);
				for (size_t K = 0, k = 0; K < N; K++, k += MM_BLOCK_SIZE)
				{
					for (T& num : a.dat)num = 0; for (T& num : b.dat)num = 0;

					const T* itA = A.dat[k] + j, * itB = B.dat[i] + k;
					T* ita = a.dat.begin(), * itb = b.dat.begin();
					// width of block of A and height of block of B
					size_t sl = min(MM_BLOCK_SIZE, n - k);
					for (size_t p = 0; p < sl; p++) {
						for (size_t q = 0; q < nl; q++)*(ita++) = *(itA++);
						itA += h - nl; ita += MM_BLOCK_SIZE - nl;
					}
					for (size_t p = 0; p < ml; p++) {
						for (size_t q = 0; q < sl; q++)*(itb++) = *(itB++);
						itB += n - sl; itb += MM_BLOCK_SIZE - sl;
					}
					MM_naive(a, b, c);
				}
				T* itC = C.dat[i] + j, * itc = c.dat.begin();
				for (size_t p = 0; p < ml; p++) {
					for (size_t q = 0; q < nl; q++)*(itC++) = *(itc++);
					itC += h - nl; itc += MM_BLOCK_SIZE - nl;
				}
			}
			i += MM_BLOCK_SIZE;
		}
	}
	template<Reals T>
	void MM_strassen(const Matrix<T>& A, const Matrix<T>& B, Matrix<T>& C)
	{
		size_t N = A.dat.width(), n = N >> 1;
		
		Assert(N == A.dat.height() && N = B.dat.width() && N == B.dat.height() && N == C.dat.width() && N == C.dat.height()
			&& N == Chromium::MathHelper::least2expGeqN(N));
		if (N <= Chromium::config::MM_BLOCK_SIZE)return MM_naive(A, B, C);
		Matrix<T> A1(n), A2(n), B1(n), B2(n);
		Matrix<T> S1(n), S2(n), S3(n), S4(n), S5(n), S6(n), S7(n), S8(n);
		Matrix<T> M1(n), M2(n), M3(n), M4(n), M5(n), M6(n), M7(n);

	}
	template<Reals T>
	void MM(const Matrix<T>& A, const Matrix<T>& B, Matrix<T>& C)
	{
		using std::max;
		using namespace Chromium::config;
		Assert(A.dat.width() == B.dat.height());
		C.dat.resize(B.dat.width(), A.dat.height());
		size_t maxe = max(C.dat.width(), C.dat.height());
		if (maxe >= 4 * MM_BLOCK_SIZE)return MM_block(A, B, C);

		MM_naive(A, B, C);
	}
}
template<Reals T>
Matrix<T> Matrix<T>::operator*(const Matrix& other)
{
	Matrix ret;
	inner::MM(*this, other, ret);
	return ret;
}
/*
export void test()
{
	using std::cout, std::endl;
	Matrix<float>A(5,5), B(4,5), C(4,5), D(4,5);
	float ni = 0;
	for (float& num : A.dat)num = ++ni;
	for (float& num : B.dat)num = ni--;
	cout << A.dat.toString() << endl << B.dat.toString() << endl;
	inner::MM_naive(A, B, C);
	inner::MM_naive2(A, B, D);
	cout << C.dat.toString() << endl << D.dat.toString();
}//*/
