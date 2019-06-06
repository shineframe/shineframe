#pragma once

#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include "../common/define.hpp"

using namespace std;

namespace shine
{
	namespace pool
	{
		template<class T>
		class simple
		{
		public:
			simple(uint32 bluck_size = 10240, bool auto_expand = true)
			{
				_bluck_size = bluck_size;
				_auto_expand = auto_expand;
			}

			~simple() {
				clear();
			}

			T* take() {
				std::unique_lock<std::recursive_mutex> lock(_mutex);
				if (_elements.empty())
				{
					if (!_auto_expand)
						return nullptr;
					expand_bluck();
				}

				T *ret = _elements.front();
				_elements.pop_front();
				return ret;
			}

			void put(T* item) {
				std::unique_lock<std::recursive_mutex> lock(_mutex);
				_elements.push_front(item);
			}

			void clear() {
				std::unique_lock<std::recursive_mutex> lock(_mutex);
				for (auto bluck : _blucks)
				{
					delete[] bluck;
				}

				_blucks.clear();
				_elements.clear();
			}

		private:
			void expand_bluck() {
				std::unique_lock<std::recursive_mutex> lock(_mutex);
				T *bluck = new T[_bluck_size];
				for (uint32 i = 0; i < _bluck_size; i++)
					_elements.push_front(bluck + i);
				_blucks.push_front(bluck);
			}

		private:
			std::forward_list<T*> _elements;
			std::forward_list<T*> _blucks;
			uint32 _bluck_size = 0;
			bool _auto_expand = true;
			std::recursive_mutex _mutex;
		};
	}

}

