#pragma once
#define mkOprWithConstruct(ty, Operator, Template) \
	template <Template T>                          \
	inline auto operator Operator(const T &right) { return *this Operator ty(right); }

#define mkOprWithConstructConst(ty, Operator, Template) \
	template <Template T>                               \
	inline auto operator Operator(const T &right) const { return *this Operator ty(right); }

#define mkOprAndAssignWithOprConcept(ty, Operator, Concept) \
	template <Concept T>                             \
	inline ty &operator Operator##=(const T & right) { return *this = *this Operator right; }

#define mkOprWithOprAndAssignConcept(ty, Operator, Concept)  \
	template <Concept T>                              \
	inline ty operator Operator(const T &right) const \
	{                                                 \
		ty ret = *this;                               \
		return ret Operator##= right;                \
	}
#define mkOprAndAssignWithOpr(ty, Operator) \
	inline ty &operator Operator##=(const ty &right) { return *this = *this Operator right; }
#define mkOprWithOprAndAssign(ty, Operator) \
	inline ty operator Operator(const ty& right) const \
	{                                                 \
	ty ret = *this;                               \
	return ret Operator## = right;                \
	}
