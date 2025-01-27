module;
#include "../base.macro.hpp"
#include <cmath>
#include <concepts>
#include <ctime>
#include <memory>
#include <random>
#include <utility>
export module Chromium.NN.base;
export import Chromium.base;
export import Chromium.NN.LA;

using Chromium::concepts::Container, Chromium::concepts::Reals;
using std::pair;
using namespace Chromium::alias;

export namespace Chromium
{
	namespace NN
	{
		float random_normal_distr(size_t idx)
		{
			static  std::mt19937 gen((unsigned int)time(nullptr));
			static std::normal_distribution<float> dis(0.0, 0.5);
			return dis(gen);
		}
		float sigmoid(float x) { return 1 / (1 + exp(-x)); }
		float deriv_sigmoid(float x)
		{
			float ex = exp(-x);
			float ret=ex / (1 + ex) / (1 + ex);
			Assert(std::isnormal(ret));
			return ret;
		}
		template<std::floating_point T>
		using tfn = T(*)(T);
		mkEmptyTag(Layer, );
		template<typename Tdat, typename Tpd_param, template<typename>typename Alloc = std::allocator>
		class LayerBase :TagT(Layer)
		{
		public:
			// copy data function
			virtual void cp(const LayerBase * other) = 0;
			// fwdprop
			virtual void run(const Tdat & in, Tdat & out)const = 0;
			/// <summary>
			///  bp=Backprop, pd=partial derivative
			/// </summary>
			/// <param name="in">input A</param>
			/// <param name="out">output B=Layer(A)</param>
			/// <param name="pd_out">=∇B</param>
			/// <param name="pd_in">=∇A</param>
			/// <param name="ppd_param">=∇Layer, it needs to be init in this function</param>
			virtual void fit_bp(const Tdat & in, const Tdat & out, const Tdat & pd_out, Tdat & pd_in, Tpd_param * ppd_param)const = 0;
			/// <summary>
			/// Layer+=η∇Layer
			/// </summary>
			/// <param name="eta">=η</param>
			/// <param name="ppd_param">=∇Layer, it needs to be destruct in this function</param>
			virtual void apply_fit(float eta, const Tpd_param * ppd_param) = 0;
			virtual Tpd_param* alloc_pdparam() = 0;
			virtual void dealloc_pdparam(Tpd_param * ppd_param) = 0;
		};
		template<template<typename>typename TCon, float fn(float), float dfn(float), template<typename>typename Alloc = std::allocator>
			requires Container<TCon>
		class Dense :LayerBase<TCon<float>, int, Alloc>
		{
			Alloc<int> alloc;
		public:
			Dense() {}
			virtual void cp(const LayerBase<TCon<float>, int, Alloc>* right)override {}
			void run(const TCon<float>& in, TCon<float>& out)const override
			{
				out = in;
				for (float& num : out)num = fn(num);
			}
			void fit_bp(const TCon<float>& in, const TCon<float>& out, const TCon<float>& pd_out, TCon<float>& pd_in, int* ppd_param)const override
			{
				Assert(in.size() == pd_out.size());
				pd_in = in;
				const float* it_pdout = pd_out.begin(), * end_pdout = pd_out.end();
				float* it_pdin = pd_in.begin();
				for (; it_pdout != end_pdout; it_pdin++, it_pdout++)
					*it_pdin = *it_pdout * dfn(*it_pdin);
			}
			void apply_fit(float eta, const int* pd_param)override {}
			int* alloc_pdparam() { return alloc.allocate(1); }
			void dealloc_pdparam(int* ppd_param)override { alloc.deallocate(ppd_param, 1); }
		};

		/*
		typedef void* (*tfn_init)(size_t inlen, size_t outlen, void* p_param);
		typedef void* (*tfn_destruct)(size_t inlen, size_t outlen, void* p_param);
		typedef (*tfn_run)(void* dat, );
		typedef void (*tfn_cp)(void* src, void* dist);
		typedef void (*tfn_fit_bp)(void* dat, const array<float>& in, const array<float>& pd_out, array<float>& pd_in, array<float>& ppd_param);
		typedef void (*tfn_apply_fit)(void* dat, const array<float>& ppd_param);
		struct Layer
		{
			size_t inlen, outlen;
			void* dat;
			tfn_init _init;
			tfn_run _run;
			tfn_fit_bp _bp;
			tfn_apply_fit _apply_fit;
			Layer(size_t inlen,size_t outlen)
		};
		struct Dense :public Layer
		{

		};*/
		template<bool bias = true, template<typename>typename Alloc = std::allocator>
		class FC :LayerBase<Matrix<float>, pair<Matrix<float>, Vector<float>>, Alloc>
		{
			using PdParam = pair<Matrix<float>, Vector<float>>;
			size_t inl, outl;
			Alloc<PdParam> alloc;
			Matrix<float> W;
			Vector<float> b;
		public:

			FC(size_t insize, size_t outsize, float(*initW_func)(size_t idx) = &random_normal_distr) :inl(insize), outl(outsize)
			{
				W.dat.resize(insize, outsize);
				if constexpr (bias)b.dat.resize(outsize);
				size_t i = 0;
				float* it = W.dat.begin(), * end = W.dat.end();
				for (; it != end; it++, i++)*it = initW_func(i);
			}
			virtual void cp(const LayerBase<Matrix<float>, pair<Matrix<float>, Vector<float>>, Alloc>* right)override
			{
				W = ((FC*)right)->W;
				if constexpr (bias)b = ((FC*)right)->b;
			}
			/// <summary>
			/// combine the multiple data as colum vectors into the matrix
			/// </summary>
			/// <param name="in">A=[v_1 v_2 ... v_n]</param>
			/// <param name="out">B=[Wv_1 Wv_2 ... Wv_n]</param>
			void run(const Matrix<float>& in, Matrix<float>& out)const override
			{
				size_t cols = in.dat.cols();
				//out.dat.resize(cols, outl);
				out = W * in;
				float* it = out.dat.begin();
				if constexpr (bias)
					for (size_t i = 0; i < cols; i++)
					{
						const float* itb = b.dat.begin(), * endb = b.dat.end();
						for (size_t j = 0; itb != endb; j++, itb++)*(it++) += *itb;
					}
			}

			void fit_bp(const Matrix<float>& in, const Matrix<float>& out, const Matrix<float>& pd_out, Matrix<float>& pd_in, PdParam* ppd_param)const override
			{
				// ∇W=(∇B)A^T ∇A=W^T(∇B)
				std::construct_at(ppd_param);
				ppd_param->first = pd_out * in.transpose();
				pd_in = W.transpose() * pd_out;
				// ∇b=Σ_{0<=i<=cols}∇B[i]
				if constexpr (bias) {
					ppd_param->second.dat.resize(b.dat.size());
					const float* itdout = pd_out.dat.begin(), * enddout = pd_out.dat.end();
					float* begindb = ppd_param->second.dat.begin(), * enddb = ppd_param->second.dat.end(), * itdb;

					while (itdout != enddout)
					{
						Assert(itdout <= enddout);
						itdb = begindb;
						for (; itdb != enddb; itdb++)*itdb += *(itdout++);
					}
				}
			}
			void apply_fit(float eta, const PdParam* ppd_param)override
			{
				/*
				if (inl == outl && inl == 16) {
					//std::cout << ppd_param->first.dat.toString();
					std::cout << W.dat.toString();
					getchar();
				}//*/
				W += ppd_param->first * eta;
				b += ppd_param->second * eta;
				//const_cast<PdParam*>(ppd_param)->first.dat.resize(0, 0);
				std::destroy_at(ppd_param);
			}
			PdParam* alloc_pdparam()override { return alloc.allocate(1); }
			void dealloc_pdparam(PdParam* ppd_param)override { alloc.deallocate(ppd_param, 1); }
		};


	}
}