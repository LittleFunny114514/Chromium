module;
#include "base.macro.hpp"
#include "NumT/NumT.macro.hpp"
export module Chromium.mathbase;
import Chromium.base;
import Chromium.NumT.base;

export namespace Chromium
{
	namespace concepts
	{
		mkTagWith(Real, empty, std::is_floating_point<T>::value);
	}
	namespace mathc
	{
		template<typename T>
		struct Pi { static const T val; };
		const double Pi<double>::val = 3.141592653589793;
		const float Pi<float>::val = 3.1415926f;
	}
}