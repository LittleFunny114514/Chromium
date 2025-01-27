module;
//*
#include "../base.macro.hpp"
#include <initializer_list>
#include <utility>//*/
export module Chromium.NN.MLP;
export import Chromium.NN.base;
using Chromium::concepts::Container, Chromium::concepts::Reals;
using std::pair, std::min, std::max;
using namespace Chromium::alias;
export namespace Chromium
{
	namespace NN
	{
		class MLP
		{
			using Tl = LayerBase<Matrix<float>, int>;
			array<Tl*>layers;
		public:
			MLP(std::initializer_list<Tl*>Layers) :layers(Layers) {}
			~MLP() {}
			// combine the multiple data as colum vectors into the matrix
			void run(const Matrix<float>& in, Matrix<float>& out)const
			{
				Matrix<float>tmp; out = in;
				for (Tl* layer : layers)layer->run(tmp = out, out);
			}
			void fit(const Matrix<float>& in, const Matrix<float>& out, ulong epoch, float eta = 0.1, size_t batch_size = 64, Matrix<float>* pd_in = nullptr)
			{
				Assert(in.dat.cols() == out.dat.cols());
				size_t l = layers.size(), inn = in.dat.rows(), outn = out.dat.rows(), datn = in.dat.cols();
				std::cout << std::endl;
				array<Matrix<float>>layers_dat(l), layers_pddat(l);
				array<int*>layers_ppdparam(l);
				for (size_t i = 0; i < l; i++)layers_ppdparam[i] = layers[i]->alloc_pdparam();
				Matrix<float>batch_in, batch_out;
				for (size_t m = 0; m < epoch; m++)
				{
					std::cout << "training... epoch " << m << ".\r";
					for (size_t I = 0; I < datn; I += batch_size)
					{
						batch_in.dat.inherit(min(batch_size, datn - I), inn, const_cast<float*>(in.dat[I]));
						batch_out.dat.inherit(min(batch_size, datn - I), outn, const_cast<float*>(out.dat[I]));

						layers[0]->run(batch_in, layers_dat[0]);
						for (size_t i = 1; i < l; i++)
							layers[i]->run(layers_dat[i - 1], layers_dat[i]);
						layers_pddat[l - 1] = batch_out - layers_dat[l - 1];
						for (size_t i = l; --i;)
							layers[i]->fit_bp(layers_dat[i - 1], layers_dat[i], layers_pddat[i], layers_pddat[i - 1], layers_ppdparam[i]);
						layers[0]->fit_bp(batch_in, layers_dat[0], layers_pddat[0], (pd_in != nullptr) ? *pd_in : layers_pddat[l - 1], layers_ppdparam[0]);
						for (size_t i = 0; i < l; i++)layers[i]->apply_fit(eta, layers_ppdparam[i]);
					}
				}
				for (size_t i = 0; i < l; i++)layers[i]->dealloc_pdparam(layers_ppdparam[i]);
				std::cout << std::endl;
			}
		};
	}
}