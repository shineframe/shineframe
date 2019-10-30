#pragma once
#include <unordered_map>
#include <vector>
#include "../util/string.hpp"

namespace shine {
	namespace db
	{
		typedef std::vector<string> row_t;
		typedef std::unordered_map<string, size_t, std::hash<std::string>, cmp_string> columns_t;
		typedef std::vector<row_t> rows_t;

		class db_result
		{
		public:
			/**
			*@brief 清空结果集
			*/
			void clear() {
				_columns.clear();
				_head.clear();
				_rows.clear();
			}
			/**
			*@brief 判断结果集中是否存在指定的列
			*@param column_name
			*@return bool
			*@warning
			*@note
			*/
			bool has_column(const string & column_name) {
				return _columns.find(column_name) != std::end(_columns);
			}

			/**
			*@brief 获取结果集中的字段数量
			*@return size_t
			*@warning
			*@note
			*/
			size_t columns_size() const {
				return _columns.size();
			}

			/**
			*@brief 获取结果集中的总行数
			*@return size_t
			*@warning
			*@note
			*/
			size_t rows_size() const {
				return _rows.size();
			}

			/**
			*@brief 获取指定行数中指定名字段的值
			*@param row_num
			*@param column_name
			*@return const string *
			*@warning
			*@note
			*/
			const string * get(size_t row_num, const string & column_name) {
				if (_rows.empty() || row_num > _rows.size() - 1 || _columns.empty())
					return nullptr;

				auto iter = _columns.find(column_name);
				if (iter == std::end(_columns) || _rows[row_num].empty() || iter->second > _rows[row_num].size() - 1)
					return nullptr;

				return &_rows[row_num][iter->second];
			}

			/**
			*@brief 获取指定行数中指定列下标的字段的值
			*@param row_num
			*@param column_num
			*@return const string *
			*@warning
			*@note
			*/
			const string * get(size_t row_num, size_t column_num) {
				if (_rows.size() == 0 || row_num > _rows.size() - 1 || _columns.size() == 0 
					|| _rows[row_num].empty() || column_num > _rows[row_num].size() - 1)
					return nullptr;

				return &_rows[row_num][column_num];
			}

			/**
			*@brief 遍历所有的字段名
			*@param func 迭代处理函数
			*@return void
			*@warning
			*@note
			*/
			void foreach_colmuns(std::function<void(size_t index, const string &name)> func) {
				if (func) {
					for (size_t i = 0; i < _head.size(); i++)
						func(i, _head[i]);
				}
			}

			/**
			*@brief 遍历指定行的所有的字段
			*@param func 迭代处理函数
			*@return void
			*@warning
			*@note
			*/
			void foreach_row(size_t row_num, std::function<void(size_t column_num, const string &column_name, const string &value)> func) {
				if (func) {
					if (_rows.empty() || row_num > _rows.size() - 1)
						return;

					row_t &row = _rows[row_num];
					for (size_t m = 0; m < row.size(); m++)
						func(m, _head[m], row[m]);
				}
			}

			/**
			*@brief 遍历结果集中所有行所有列的数据
			*@param func 迭代处理函数
			*@return void
			*@warning
			*@note
			*/
			void foreach_rows(std::function<void(size_t row, size_t column_num, const string &column_name, const string &value)> func) {
				if (func) {
					for (size_t i = 0; i < _rows.size(); i++)
					{
						row_t &row = _rows[i];
						for (size_t m = 0; m < row.size(); m++)
							func(i, m, _head[m], row[m]);
					}
				}
			}	

		protected:
			SHINE_GEN_MEMBER_GETSET(columns_t, columns);
			SHINE_GEN_MEMBER_GETSET(row_t, head);
			SHINE_GEN_MEMBER_GETSET(rows_t, rows);

		};
	}
}