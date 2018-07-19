#include <iostream>
#include "mysql/mysql.hpp"

using namespace shine;
using namespace shine::db;

int main(){
    db::mysql_connect_info connect_info;
    connect_info.set_addr("172.10.4.19:3306");
    connect_info.set_user("root");
    connect_info.set_password("root");
    connect_info.set_database("tmp");
    shine::db::mysql_pool pool(connect_info);
    auto conn = pool.get();

    conn->execute("DROP TABLE IF EXISTS students");
    conn->execute("CREATE TABLE `students` (\
        `id`  int(11) NOT NULL AUTO_INCREMENT,\
        `name`  varchar(32) NOT NULL,\
        `age`  int(4) NOT NULL,\
        PRIMARY KEY(`id`)\
        )");

    shine::string sql = "INSERT INTO `students` (`id`, `name`, `age`) VALUES ";
    for (int i = 1; i <= 100; i++)
    {
        if (i > 1)
            sql.append(",");

        sql.format_append("(%d, 'name_%d', %d) ", i, i, i);
    }

    conn->execute(sql);

    db::mysql_result result;
    conn->select("SELECT * FROM students", &result);

    result.foreach_colmuns([](shine::size_t index, const shine::string &name){
        std::cout << name << "    ";
    });

    std::cout << std::endl;

    shine::size_t row_num = -1;
    result.foreach_rows([&row_num](shine::size_t row, shine::size_t column_num, const shine::string &column_name, const shine::string &value){
        if (row_num != row)
        {
            row_num = row;
            std::cout << std::endl;
        }

        std::cout<< value << "    ";
    });
    return 0;
}
