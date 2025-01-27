module;
#include "../base.macro.hpp"
#include "../NumT/NumT.macro.hpp"
#include <concepts>
#include <utility>
export module Chromium.NN.LA;
import Chromium.base;
export import Chromium.mathbase;
export import Chromium.utilities;
using Chromium::concepts::Arithmetic, Chromium::pos2;
export namespace Chromium
{
	namespace config
	{
		const size_t MM_BLOCK_SIZE = 64;
		const size_t MM_MIN_STRASSEN = 256;
	}
	template<Arithmetic T>
	class Vector
	{
	public:
		array<T> dat;
		Vector() {}
		Vector(size_t dim, T fill = T()) { dat.resize(dim, fill); }
		Vector(const Vector& right) {dat = right.dat; }
		Vector& operator=(const Vector& right) { return dat = right.dat, *this; }
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
		mkOprWithOprAndAssign(Vector, +);
		mkOprWithOprAndAssign(Vector, -);
		// dot product
		T operator*(const Vector& other)
		{
			Assert(dat.size() == other.dat.size());
			size_t i = 0; T ret = T();
			for (size_t i = 0; i < dat.size(); i++)
				ret += dat[i] * other.dat[i];
			return *this;
		}
		Vector& operator*=(T lambda)
		{
			T* it = dat.begin(), * end = dat.end();
			while (it != end) *(it++) *= lambda;
			return *this;
		}
		inline Vector operator*(T lambda)const
		{
			Vector ret = *this;
			ret *= lambda;
			return ret;
		}
	};
	template<Arithmetic T>
	class Matrix
	{
	public:
		array2d<T>dat;
		Matrix() {}
		Matrix(size_t w, size_t h, T fill = T()) { dat.resize(w, h, fill); }
		Matrix(size_t sidelen) { dat.resize(sidelen, sidelen, T()); }
		Matrix(const Matrix& right) { dat = right.dat; }
		Matrix& operator=(const Matrix& right) { return dat = right.dat, *this; }

		Matrix& operator+=(const Matrix& other)
		{
			Assert(dat.cols() == other.dat.cols() && dat.rows() == other.dat.rows());
			T* it = dat.begin(), * end = dat.end();
			const T* ito = other.dat.begin();
			while (it != end) *(it++) += *(ito++);
			return *this;
		}
		Matrix& operator-=(const Matrix& right)
		{
			Assert(dat.cols() == right.dat.cols() && dat.rows() == right.dat.rows());
			T* it = dat.begin(), * end = dat.end();
			const T* ito = right.dat.begin();
			while (it != end) *(it++) -= *(ito++);
			return *this;
		}
		Matrix& operator-()
		{
			Matrix ret = *this;
			T* it = ret.dat.begin(), * end = ret.dat.end();
			while (it != end) *it = -*it, it++;
			return *this;
		}
		Matrix operator*(const Matrix&)const;
		mkOprWithOprAndAssign(Matrix, +);
		mkOprWithOprAndAssign(Matrix, -);
		mkOprAndAssignWithOpr(Matrix, *);
		Matrix& operator*=(T lambda)
		{
			T* it = dat.begin(), * end = dat.end();
			while (it != end) *(it++) *= lambda;
			return *this;
		}
		inline Matrix operator*(T lambda)const
		{
			Matrix ret = *this;
			ret *= lambda;
			return ret;
		}
		Matrix transpose()const
		{
			size_t rows, cols;
			Matrix ret(rows = dat.rows(), cols = dat.cols());
			for (size_t i = 0; i < cols; i++)
				for (size_t j = 0; j < rows; j++)ret.dat[j][i] = dat[i][j];
			return ret;
		}


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
	template<Arithmetic T>
	void MM_naive(const Matrix<T>& A, const Matrix<T>& B, Matrix<T>& C)
	{
		using namespace Chromium::alias;
		size_t w = C.dat.cols(), h = C.dat.rows(), n = A.dat.cols();
		const T* itb = B.dat.begin(), * ita;
		T* pivc = C.dat.begin(), * itc;
		for (size_t i = 0; i < w; i++)
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
	template<Arithmetic T, void(*BlockMM)(const Matrix<T>&, const Matrix<T>&, Matrix<T>&) = MM_naive>
	void MM_block(const Matrix<T>& A, const Matrix<T>& B, Matrix<T>& C, size_t blocksize = Chromium::config::MM_BLOCK_SIZE)
	{
		using namespace Chromium::alias;
		using std::max, std::min;
		size_t w = C.dat.cols(), h = C.dat.rows(), n = A.dat.cols();
		size_t W = divceil(w, blocksize), H = divceil(h, blocksize), N = divceil(n, blocksize);

		//#pragma omp parallel for
		for (llong I = 0; I < W; I++)
		{
			size_t i = I * blocksize;
			for (size_t J = 0, j = 0; J < H; J++, j += blocksize)
			{
				Matrix<T> a(blocksize), b(blocksize), c(blocksize);
				// width and height of blocks of product
				size_t ml = min(blocksize, w - i), nl = min(blocksize, h - j);
				for (size_t K = 0, k = 0; K < N; K++, k += blocksize)
				{
					for (T& num : a.dat)num = 0; for (T& num : b.dat)num = 0;

					const T* itA = A.dat[k] + j, * itB = B.dat[i] + k;
					T* ita = a.dat.begin(), * itb = b.dat.begin();
					// width of block of A and height of block of B
					size_t sl = min(blocksize, n - k);
					for (size_t p = 0; p < sl; p++) {
						for (size_t q = 0; q < nl; q++)*(ita++) = *(itA++);
						itA += h - nl; ita += blocksize - nl;
					}
					for (size_t p = 0; p < ml; p++) {
						for (size_t q = 0; q < sl; q++)*(itb++) = *(itB++);
						itB += n - sl; itb += blocksize - sl;
					}
					MM_naive(a, b, c);
				}
				T* itC = C.dat[i] + j, * itc = c.dat.begin();
				for (size_t p = 0; p < ml; p++) {
					for (size_t q = 0; q < nl; q++)*(itC++) = *(itc++);
					itC += h - nl; itc += blocksize - nl;
				}
			}
		}
	}
	template<Arithmetic T>
	void MM_strassenRecursion(const Matrix<T>& A, const Matrix<T>& B, Matrix<T>& C, T* p)
	{
		size_t N = A.dat.cols(), n = N >> 1;

		if (N <= Chromium::config::MM_MIN_STRASSEN)
			return Chromium::fill<T>(C.dat.begin(), C.dat.size(), 0), MM_naive(A, B, C);
		// temp matrices
		Matrix<T> A1, A2, B1, B2;
		Matrix<T> S1, S2, S3, S4, S5, S6, S7, S8, S9, S10;
		Matrix<T> M1, M2, M3, M4, M5, M6, M7;
		A1.dat.inherit(n, n, p), A2.dat.inherit(n, n, p + n * n), B1.dat.inherit(n, n, p + 2 * n * n), B2.dat.inherit(n, n, p + 3 * n * n);
		S1.dat.inherit(n, n, p + 4 * n * n), S2.dat.inherit(n, n, p + 5 * n * n), S3.dat.inherit(n, n, p + 6 * n * n), S4.dat.inherit(n, n, p + 7 * n * n), S5.dat.inherit(n, n, p + 8 * n * n);
		S6.dat.inherit(n, n, p + 9 * n * n), S7.dat.inherit(n, n, p + 10 * n * n), S8.dat.inherit(n, n, p + 11 * n * n), S9.dat.inherit(n, n, p + 12 * n * n), S10.dat.inherit(n, n, p + 13 * n * n);
		M1.dat.inherit(n, n, p + 14 * n * n), M2.dat.inherit(n, n, p + 15 * n * n), M3.dat.inherit(n, n, p + 16 * n * n), M4.dat.inherit(n, n, p + 17 * n * n),
			M5.dat.inherit(n, n, p + 18 * n * n), M6.dat.inherit(n, n, p + 19 * n * n), M7.dat.inherit(n, n, p + 20 * n * n);
		// iterator of the 8 submatrices l=left,r=rigtht,t=top,b=bottom
		const T* Alt = A.dat.begin(), * Alb = Alt + n, * Art = Alt + N * n, * Arb = Alb + N * n,
			* Blt = B.dat.begin(), * Blb = Blt + n, * Brt = Blt + N * n, * Brb = Blb + N * n;
		// iterator of temp matrices
		T* ita1 = A1.dat.begin(), * ita2 = A2.dat.begin(), * itb1 = B1.dat.begin(), * itb2 = B2.dat.begin(),
			* its1 = S1.dat.begin(), * its2 = S2.dat.begin(), * its3 = S3.dat.begin(), * its4 = S4.dat.begin(), * its5 = S5.dat.begin(),
			* its6 = S6.dat.begin(), * its7 = S7.dat.begin(), * its8 = S8.dat.begin(), * its9 = S9.dat.begin(), * its10 = S10.dat.begin();
		for (size_t i = 0; i < n; i++)
		{
			for (size_t j = 0; j < n; j++)
			{
				*ita1 = *Alt, * ita2 = *Arb, * itb1 = *Blt, * itb2 = *Brb;
				*its1 = *Alt + *Arb, * its2 = *Alb + *Arb, * its3 = *Alt + *Art, * its4 = *Alb - *Alt, * its5 = *Art - *Arb;
				*its6 = *Blt + *Brb, * its7 = *Brt - *Brb, * its8 = *Blb - *Blt, * its9 = *Blt + *Brt, * its10 = *Blb + *Brb;
				ita1++, ita2++, itb1++, itb2++;
				its1++, its2++, its3++, its4++, its5++, its6++, its7++, its8++, its9++, its10++;
				Alt++, Alb++, Art++, Arb++, Blt++, Blb++, Brt++, Brb++;
			}
			Alt += n, Alb += n, Art += n, Arb += n, Blt += n, Blb += n, Brt += n, Brb += n;
		}

		T* pnext = p + 21 * n * n;
		MM_strassenRecursion(S1, S6, M1, pnext), MM_strassenRecursion(S2, B1, M2, pnext), MM_strassenRecursion(A1, S7, M3, pnext), MM_strassenRecursion(A2, S8, M4, pnext),
			MM_strassenRecursion(S3, B2, M5, pnext), MM_strassenRecursion(S4, S9, M6, pnext), MM_strassenRecursion(S5, S10, M7, pnext);

		T* Clt = C.dat.begin(), * Clb = Clt + n, * Crt = Clt + N * n, * Crb = Clb + N * n;
		T* itm1 = M1.dat.begin(), * itm2 = M2.dat.begin(), * itm3 = M3.dat.begin(), * itm4 = M4.dat.begin(), * itm5 = M5.dat.begin(), * itm6 = M6.dat.begin(), * itm7 = M7.dat.begin();
		for (size_t i = 0; i < n; i++)
		{
			for (size_t j = 0; j < n; j++)
			{
				*Clt = *itm1 + *itm4 - *itm5 + *itm7;
				*Crb = *itm1 - *itm2 + *itm3 + *itm6;
				*Clb = *itm2 + *itm4, * Crt = *itm3 + *itm5;
				itm1++, itm2++, itm3++, itm4++, itm5++, itm6++, itm7++;
				Clt++, Clb++, Crt++, Crb++;
			}
			Clt += n, Clb += n, Crt += n, Crb += n;
		}
	}
	template<Arithmetic T>
	void MM_strassen(const Matrix<T>& A, const Matrix<T>& B, Matrix<T>& C)
	{
		static const size_t mins = Chromium::config::MM_MIN_STRASSEN;
		size_t N = A.dat.cols();
		Assert(N == A.dat.rows() && N == B.dat.cols() && N == B.dat.rows() && N == C.dat.cols() && N == C.dat.rows()
			&& N == Chromium::MathHelper::least2expGeqN(N));
		if (N <= mins)return MM_naive(A, B, C);
		T* p = new T[(N * N - mins * mins) * 7];
		MM_strassenRecursion(A, B, C, p);
		delete[] p;
	}
	template<Arithmetic T>
	void MM(const Matrix<T>& A, const Matrix<T>& B, Matrix<T>& C)
	{
		using std::max;
		using namespace Chromium::config;
		Assert(A.dat.cols() == B.dat.rows());
		C.dat.resize(B.dat.cols(), A.dat.rows());
		size_t maxe = max(C.dat.cols(), C.dat.rows());


		if (maxe >= 4 * MM_BLOCK_SIZE)return MM_block(A, B, C);
		MM_naive(A, B, C);
	}
}
template<Arithmetic T>
Matrix<T> Matrix<T>::operator*(const Matrix& right)const
{
	Matrix ret;
	inner::MM(*this, right, ret);
	return ret;
}
/*
export void test()
{
	using std::cout, std::endl;
	Matrix<int>A(16), B(16), C(16), D(16);
	int ni = 0;
	for (int& num : A.dat)num = rand()%4;
	for (int& num : B.dat)num = rand()%4;
	cout << A.dat.toString() << endl << B.dat.toString() << endl;
	inner::MM_naive(A, B, C);
	inner::MM_strassen(A, B, D);
	cout << C.dat.toString() << endl << D.dat.toString();
}//*/
