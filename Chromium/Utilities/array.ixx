module;
#include<assert.h>
export module Chromium.utilities;
import std;
import Chromium.base;
export namespace Chromium
{
	template <typename T>
	class array
	{
		T* _data = nullptr;
		size_t _size = 0;
		T _cache[config::MAX_ARR_CACHE_SIZE] = { T() };

	public:
		array() {}
		array(size_t Size, T fill = T()) { resize(Size, fill); }
		array(std::initializer_list<T> init)
		{
			if (!init.size())
				return;
			T* it;
			_size = init.size();
			if (init.size() <= config::MAX_ARR_CACHE_SIZE)it = _cache;
			else
				it = _data = new T[_size];
			for (const T& t : init)
				*(it++) = t;
		}
		array(const array& other) { *this = other; }
		~array()
		{
			if (_data != nullptr)
				delete[] _data;
		}
		array& resize(size_t newsize, T fill = T())
		{
			if (_data)
				delete[] _data;
			if (!newsize)
				return _data = nullptr, _size = 0, *this;

			_data = (newsize <= config::MAX_ARR_CACHE_SIZE ? 0 : new T[newsize]);
			_size = newsize;
			T* it = begin(), * End = end();
			while (it != End)
				*(it++) = fill;
			return *this;
		}
		inline array& clear() { return resize(0); }

		inline T& operator[](size_t idx)
		{
			assert(idx < _size);
			return begin()[idx];
		}
		inline const T& operator[](size_t idx) const
		{
			assert(idx < _size);
			return begin()[idx];
		}
		inline T& at(size_t idx) { return operator[](idx); }
		inline const T& at(size_t idx)const { return operator[](idx); }


		inline T* begin() { return _data ? _data : _cache; }
		inline T* end() { return begin() + _size; }
		inline const T* begin() const { return _data ? _data : _cache; }
		inline const T* end() const { return begin() + _size; }
		inline size_t size() const { return _size; }
		inline const T* data() const { return begin(); }

		bool operator==(const array& other) const
		{
			if (_size != other._size)
				return false;
			const T* it1 = begin(), * it2 = other.begin(), * end1 = end();
			while (it1 != end1)
				if (*(it1++) != *(it2++))
					return false;
			return true;
		}
		inline bool operator!=(const array& other) const { return !operator==(other); }
		array& operator=(const array& other)
		{
			if (_data)
				delete[] _data;
			if (!other._size)
				return _size = 0, _data = nullptr, *this;
			_data = (other._size <= config::MAX_ARR_CACHE_SIZE ? 0 : new T[other._size]);
			_size = other._size;
			T* it1 = begin(), * end1 = end();
			const T* it2 = other.begin();
			while (it1 != end1)
				(*it1++) = (*it2++);
			return *this;
		}
	};
}