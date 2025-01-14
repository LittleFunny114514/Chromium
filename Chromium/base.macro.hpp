#pragma once
#ifdef _CR_MSVE_USED
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
// template<typename T>inline T max(const T& a, const T& b) { return (a > b) ? a : b; }
// template<typename T>inline T min(const T& a, const T& b) { return (a < b) ? a : b; }
#endif //_CR_MSVC_USED
#include<limits.h>
#include <assert.h>
#define Assert assert
#define comma ,
#define empty
#pragma warning(disable:5276)
#define mkEmptyTag(name, super) \
	class tag_##name super { };\
	template <typename T>\
	concept name##s = std::is_base_of<tag_##name, T>::value;
#define mkTagWith(name, super, condition) \
	class tag_##name super { };\
	template <typename T>\
	concept name##s = std::is_base_of<tag_##name, T>::value || condition;
#define mkConcept1Type(type) template <typename T> \
									concept Only_##type = std::is_same_v<T, type>
#define TagT(name) tag_##name