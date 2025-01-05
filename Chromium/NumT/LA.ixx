module;
#include "../base.macro.hpp"
#include "NumT.macro.hpp"
export module LA;
import Chromium.base;
import Chromium.mathbase;
import Chromium.utilities;
export namespace Chromium
{
	template<concepts::Reals T>
	class dvector
	{
		size_t  size = 0;
		array<T> dat;
	public:
		inline void resize(size_t dim) { dat.resize(size = dim); }
	};
}