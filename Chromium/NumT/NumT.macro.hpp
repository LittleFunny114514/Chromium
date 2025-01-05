#pragma once
#define mkOprWithConstruct(ty, Operator, Template) \
	template <Template T>                          \
	inline auto operator Operator(const T &other) { return *this Operator ty(other); }

#define mkOprWithConstructConst(ty, Operator, Template) \
	template <Template T>                               \
	inline auto operator Operator(const T &other) const { return *this Operator ty(other); }

#define mkOprAndAssignWithOpr(ty, Operator, Concept) \
	template <Concept T>                             \
	inline ty &operator Operator##=(const T & other) { return *this = *this Operator other; }

#define mkOprWithOprAndAssign(ty, Operator, Concept)  \
	template <Concept T>                              \
	inline ty operator Operator(const T &other) const \
	{                                                 \
		ty ret = *this;                               \
		return ret Operator##= other;                \
	}
