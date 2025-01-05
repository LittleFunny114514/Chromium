module;
#include "../base.macro.hpp"
#include "NumT.macro.hpp"
export module Chromium.NumT.base;
export import Chromium.base;
import Chromium.utilities;

export namespace Chromium
{
	namespace concepts
	{
		mkEmptyTag(hiInt, empty);
		template <typename T>
		concept Ints = hiInts<T> || std::integral<T>;
		mkEmptyTag(CR_complex,);
	}
	namespace alias
	{
		typedef ulong Int_cellT;  // 长整型的数据存储单位
		typedef ullong Int_calcT; // 长整型计算每一位（数据存储单位）时运用的数据类型
	}
	namespace config
	{
		using namespace alias;
		
		const size_t MIN_SIZE_OF_INT = 4;
		const size_t MAX_SIZE_PRIMARY_PROCESS = 0x400;
		typedef array<config::Int_cellT> hInt_datT;
	}
}
