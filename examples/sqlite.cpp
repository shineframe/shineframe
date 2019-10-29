#include <iostream>
#include "sqlite/sqlite.hpp"

using namespace shine;
using namespace shine::db;

int main(){

	db::sqlite obj;
	obj.open("1.db");

	shine::string sql = "DELETE FROM COMPANY;";

	std::cout << obj.execute(sql) << std::endl;

	sql = "CREATE TABLE IF NOT EXISTS COMPANY("  \
		"ID INT PRIMARY KEY     NOT NULL," \
		"NAME           TEXT    NOT NULL," \
		"AGE            INT     NOT NULL," \
		"ADDRESS        CHAR(50)," \
		"SALARY         REAL );";

	std::cout << obj.execute(sql) << std::endl;

	sql = "INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
		"VALUES (1, 'Paul', 32, 'California', 20000.00 ); " \
		"INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY) "  \
		"VALUES (2, 'Allen', 25, 'Texas', 15000.00 ); "     \
		"INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
		"VALUES (3, 'Teddy', 23, 'Norway', 20000.00 );" \
		"INSERT INTO COMPANY (ID,NAME,AGE,ADDRESS,SALARY)" \
		"VALUES (4, 'Mark', 25, 'Rich-Mond ', 65000.00 );";

	std::cout << obj.execute(sql) << std::endl;

	sql = "SELECT * from COMPANY";

	shine::db::sqlite_result result;
	obj.select(sql, result);

	std::cout << obj.execute(sql) << std::endl;

	shine::size_t row_num = -1;
	result.foreach_rows([&row_num](shine::size_t row, shine::size_t column_num, const shine::string &column_name, const shine::string &value) {
		if (row_num != row)
		{
			row_num = row;
			std::cout << std::endl;
		}

		std::cout << value << "    ";
	});

	return 0;
}
