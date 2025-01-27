module;
#include "../base.macro.hpp"
export module Chromium.utilities;
import Chromium.base;
export namespace Chromium
{
	template<typename T>
	struct pos2
	{
		T x, y;
		pos2(T X, T Y) :x(X), y(Y) {}
	};
	template <typename T, template<typename>typename Alloc = std::allocator>
	class array
	{
		bool ext_alloc = false;
		Alloc<T> alloc;
		T* _data = nullptr;
		size_t __unused1, __unused2, _size = 0;
		T cache[config::MAX_ARR_CACHE_SIZE] = { T() };
	public:
		array() {}
		array(size_t Size, T fill = T()) { resize(Size, fill); }
		array(std::initializer_list<T> init)
		{
			if (!init.size())
				return;
			T* it;
			_size = init.size();
			if (init.size() <= config::MAX_ARR_CACHE_SIZE)it = cache;
			else
				it = _data = alloc.allocate(_size);
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
				alloc.deallocate(_data, _size);
			if (!newsize)
				return _data = nullptr, _size = 0, *this;

			_data = (newsize <= config::MAX_ARR_CACHE_SIZE ? 0 : alloc.allocate(newsize));
			_size = newsize;
			T* it = begin(), * End = end();
			while (it != End)
				*(it++) = fill;
			return *this;
		}
		inline array& clear() { return resize(0); }

		inline T& operator[](size_t idx)
		{
			Assert(idx < _size);
			return begin()[idx];
		}
		inline const T& operator[](size_t idx) const
		{
			Assert(idx < _size);
			return begin()[idx];
		}
		inline T& at(size_t idx) { return operator[](idx); }
		inline const T& at(size_t idx)const { return operator[](idx); }

		inline T* begin() { return _data ? _data : cache; }
		inline T* end() { return begin() + _size; }
		inline const T* begin() const { return _data ? _data : cache; }
		inline const T* end() const { return begin() + _size; }
		inline size_t size() const { return _size; }
		inline T* data() { return begin(); }
		inline const T* data()const { return begin(); }

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
				alloc.deallocate(_data, _size);
			if (!other._size)
				return _size = 0, _data = nullptr, *this;
			_data = (other._size <= config::MAX_ARR_CACHE_SIZE ? 0 : alloc.allocate(other._size));
			_size = other._size, __unused1 = other.__unused1, __unused2 = other.__unused2;
			T* it1 = begin(), * end1 = end();
			const T* it2 = other.begin();
			while (it1 != end1)
				*(it1++) = *(it2++);
			return *this;
		}
		std::string toString(std::string split = " ")
		{
			std::ostringstream oss;
			for (const T& e : *this)
				oss << e << split;
			return oss.str();
		}
	};
	template<typename T, template<typename>typename Alloc = std::allocator>
	class array2d
	{
		bool ext_alloc = false;
		Alloc<T> alloc;
		T* _data = nullptr;
		size_t cx = 0, cy = 0, _size = 0;
		T cache[config::MAX_ARR_CACHE_SIZE] = { T() };
		//typedef T(*GenFn)(T);
	public:
		inline size_t cols()const { return cx; }
		inline size_t rows()const { return cy; }
		inline size_t size()const { return _size; }
		array2d() {}
		array2d(size_t cols, size_t rows) { resize(cols, rows); }
		array2d(size_t cols, size_t rows, T* data) { inherit(cols, rows, data); }
		array2d& operator=(const array2d& other)
		{
			if (!other._size)return resize(0, 0);
			resize(other.cx, other.cy);
			T* it1 = begin(), * end1 = end();
			const T* it2 = other.begin();
			while (it1 != end1)
				*(it1++) = *(it2++);
			return *this;
		}
		void inherit(size_t cols, size_t rows, T* data)
		{
			if (_data != nullptr && !ext_alloc)alloc.deallocate(_data, _size);
			cx = cols, cy = rows, _size = cx * cy, ext_alloc = true, _data = data;
		}
		void deinherit() { Assert(ext_alloc); ext_alloc = false, _data = nullptr; }
		~array2d() {
			if (_data != nullptr && !ext_alloc)alloc.deallocate(_data, _size);
		}
		array2d& resize(size_t cols, size_t rows, T fill = T())
		{
			Assert(!ext_alloc);
			if (_data != nullptr)alloc.deallocate(_data, _size);
			_size = (cx = cols) * (cy = rows);
			if (!_size)return _data = nullptr, *this;
			_data = (_size <= config::MAX_ARR_CACHE_SIZE ? 0 : alloc.allocate(_size));
			T* it = begin(), * End = end();
			while (it != End)
				*(it++) = fill;
			return *this;
		}
		inline T* begin()
		{
			Assert(_size);
			return _data ? _data : cache;
		}
		inline T* end() { return begin() + _size; }
		inline const T* begin()const
		{
			Assert(_size);
			return _data ? _data : cache;
		}
		inline const T* end()const { return begin() + _size; }
		inline T* operator[](size_t x)
		{
			Assert(x < cx);
			return &(begin()[x * cy]);
		}
		inline const T* operator[](size_t x)const
		{
			Assert(x < cx);
			return &(begin()[x * cy]);
		}
		std::string toString(std::string sep = " ", std::string endl = "\n")const
		{
			std::ostringstream oss;
			for (size_t j = 0; j < cy; j++)
			{
				for (size_t i = 0; i < cx; i++)oss << begin()[i * cy + j] << sep;
				oss << endl;
			}
			return oss.str();
		}
	};
}