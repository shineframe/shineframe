# shineframe : 高性能超轻量级C++开发库及服务器编程框架
![](https://i.imgur.com/A78rQ4O.png)

[![Open Source Love](https://badges.frapsoft.com/os/v1/open-source.svg?v=103)](https://github.com/ellerbrock/open-source-badge/)
[![stable](http://badges.github.io/stability-badges/dist/stable.svg)](http://github.com/badges/stability-badges)[![Build Status](https://travis-ci.com/shineframe/shineframe.svg?branch=master)](https://travis-ci.com/shineframe/shineframe)

## 一、简述 ## 
shineframe是使用c++11编写，支持(linux/windows)平台。用户通过它可以非常方便地开发非阻塞式高并发服务器程序，同时提供了一些功能库，使开发的过程变得简单高效。

shineframe使用时只需要包含.hpp头文件即可，编译时需要添加 -std=c++11选项。目前处于不断开发的过程中，功能库将会逐步丰富及持续改进。

github：[https://github.com/shineframe/shineframe](https://github.com/shineframe/shineframe)

技术交流群：805424758

![](https://i.imgur.com/dHd7VJZ.png)

## 二、库组成 ##


### [shine serial](https://github.com/shineframe/shine_serial "shine_serial") 媲美protobuf的强大序列化/反序列化工具 ###

支持c++原生对象的序列化与反序列化，是网络自定义协议格式应用的开发利器。

shine serial编解效率均高于google protobuf,提供与protobuf相似的序列化特性，如：数值压缩编码，类似于varint,序列化后体积极小(小于protobuf)。serial支持协议向前向后兼容（新版本的model能够解码旧版本的字节流，旧版本的model也能够解码新版本的字节流），同时serial支持比protobuf更丰富强大的数据类型，基本的数据类型及主要STL标准容器类型字段均可进行序列化(vector, deque, list, forward_list, map, unordered_map, set, unordered_set)，支持结构嵌套（注:嵌套的结构体一定也要以SHINE_SERIAL宏修饰，否则不支持，编译不通过）

shine_serial操作示例（一行代码实现c++原生对象的序列化与反序列化）：

	
	#include <iostream>
	#include "shine_serial.hpp"

	struct B
	{
	    int a;
	    double b;
	    std::string c;
	    //将类型名称及需要序列化的字段用SHINE_SERIAL包裹
	    SHINE_SERIAL(B, a, b, c);
	};
	
	struct A{
	    int a;
	    double b;
	    std::string c;
	
	    //此处嵌套上方的结构体B
	    std::map<int, B> d;
	
	    std::list<int> e;
	    std::vector<float> f;
	    std::deque<double> g;
	    std::forward_list<long> h;
	    std::set<std::string> i;
	
	    SHINE_SERIAL(A, a, b, c, d, e, f, g, h, i);
	};
	
	int main(){
	
	    A a;
	    a.a = 123;
	    a.b = 345.567;
	    a.c = "hello world!";
	
	    B b;
	
	    b.a = 666;
	    b.b = 777.7777;
	    b.c = "999999!";
	
	    a.d.emplace(999, b);
	
	    a.e.emplace_back(123);
	    a.e.emplace_back(345);
	
	    a.f.emplace_back((float)12.34);
	    a.f.emplace_back((float)45.67);
	
	    a.g.emplace_back((double)456.789);
	    a.g.emplace_back((double)78.9);
	
	    a.h.emplace_front(666);
	    a.h.emplace_front(555);
	
	    a.i.emplace("A");
	    a.i.emplace("B");
	    a.i.emplace("C");
	
	    //将对象a序列化成字节流
	    auto data = a.shine_serial_encode();
	
	    //将字节流反序列化成对象，反序列化后a2与a中数据相同
	    A a2;
	    a2.shine_serial_decode(data);
	
        //确定结果
	    std::cout << ((a == a2) ? "success" : "failed") << std::endl;
	
	    return 0;
	}


执行后输出:success


### json 强大的json工具 ###

json解析类，支持字符串与json对象的互转，另外支持json字符串与c++原生对象的互转，是json协议格式应用的开发利器。

json_model操作示例（一行代码实现json字符串与c++原生对象的互转）：

	
	#include <iostream>
	#include "util/json.hpp"
	
	using namespace shine;
	
	struct B
	{
	    int a;
	    double b;
	    std::string c;
	    SHINE_JSON_MODEL(B, a, b, c);
	};
	
	struct A{
	    int a;
	    double b;
	    std::string c;
	
	    std::map<int, B> d;
	    std::list<int> e;
	    std::vector<float> f;
	    std::deque<double> g;
	    std::forward_list<long> h;
	    std::set<shine::string> i;
	
	    SHINE_JSON_MODEL(A, a, b, c, d, e, f, g, h, i);
	};
	
	int main(){
	
	    A a;
	    a.a = 123;
	    a.b = 345.567;
	    a.c = "hello world!";
	
	    B b;
	
	    b.a = 666;
	    b.b = 777.7777;
	    b.c = "999999!";
	
	    a.d.emplace(999, b);
	
	    a.e.emplace_back(123);
	    a.e.emplace_back(345);
	
	    a.f.emplace_back((float)12.34);
	    a.f.emplace_back((float)45.67);
	
	    a.g.emplace_back((double)456.789);
	    a.g.emplace_back((double)78.9);
	
	    a.h.emplace_front(666);
	    a.h.emplace_front(555);
	
	    a.i.emplace("A");
	    a.i.emplace("B");
	    a.i.emplace("C");
	
	    //将对象a编码成json字符串
	    auto a_str = a.json_encode();
	
	    //将json字符串解码成对象，解码后a2与a中数据相同
	    A a2;
	    a2.json_decode(a_str);
	
	    return 0;
	}


json基本操作示例：


	#include <iostream>
	#include "../json.hpp"
	
	using namespace shine;
	
	int json_base_main(){
	
	    shine::string json_str = "{\
	        \"name\": \"\\\"BeJson\\\"\",\
	        \"url\": \"http://www.bejson.com\",\
	        \"page\": 33.1288,\
	        \"isNonProfit\": true,\
	        \"address\": {\
	        \"street\": \"科技园路.\",\
	        \"city\": \"江苏苏州\",\
	        \"country\": \"中国\"\
	},\
	\"links\": [\
	    {\
	        \"name\": \"Google\",\
	            \"url\": \"http://www.google.com\"\
	    },\
	    {\
	        \"name\": \"Baidu\",\
	        \"url\": \"http://www.baidu.com\"\
	    },\
	    {\
	        \"name\": \"SoSo\",\
	        \"url\": \"http://www.SoSo.com\"\
	    }\
	    ]\
	}";
	
	    shine::json json;
	    json.decode(json_str);
	
	    shine::string json_str_encode = json.encode();
	
	    shine::json json2;
	
	    json_node_t node1;
	    node1.set_key("node1");
	    node1.set_number("-123.345");
	    json2.get_root().insert_kv_child(node1);
	
	    auto json2_str = json2.encode();
	
	    shine::json json3;
	    json_node_t arr;
	    arr.set_key("arr");
	
	    for (int i = 0; i < 5; i++)
	    {
	        json_node_t tmp;
	        tmp.set_string(shine::string(i));
	        arr.push_back_array_child(tmp);
	    }
	
	    json3.get_root().insert_kv_child(arr);
	    auto json3_str = json3.encode();
	
	
	    return 0;
	}


### string 字符串封装 ###

### log 简单的日志实现 ###

### timer 定时器实现 ###

### pool 简单的对象池实现 ###

### redis redis客户端封装 ###
目前只实现了同步式请求，异步式请求与请求/发布功能待完善。

简单同步客户端代码示例：

	#include <iostream>
	#include <set>
	#include <vector>
	#include <map>
	#include "../redis.hpp"
	
	using namespace shine;
	using namespace shine::net;
	
	int redis_main(){
	    shine::redis::sync redis;
	    redis.set_addr("127.0.0.1:6379");
	    redis.set_recv_timeout(3000);
	    redis.set_auth("password");
	
	    shine::string str;
	    std::vector<shine::string> arr;
	    std::set<shine::string> set;
	    std::map<shine::string, shine::string> map;
	
	    redis.SET("library", "redis");
	    redis.GET("library", str);
	    std::cout << str << std::endl;
	
	
	    redis.SADD("names", "a", "b", "c", "e");
	    redis.SMEMBERS("names", set);
	    for (auto &iter : set){
	        std::cout << iter << std::endl;
	    }
	
	    redis.KEYS("*", arr);
	    for (auto &iter : arr){
	        std::cout << iter << std::endl;
	    }
	
	    redis.HSET("members", "a", "A");
	    redis.HSET("members", "b", "B");
	    redis.HSET("members", "c", "C");
	    redis.HGETALL("members", map);
	    for (auto &iter : map){
	        std::cout << iter.first << ":" << iter.second << std::endl;
	    }
	
	    return 0;
	}


### net 网络封装 ###
主要封装了socket操作，提供proactor风格非阻塞套接字操作。

echo服务端示例：

    #include <iostream>
	#include "../proactor_engine.hpp"
	
	using namespace shine;
	using namespace shine::net;
	
	int echo_server_main(){
	
	    std::cout << "bind address: ";
	    shine::string addr;
	    std::cin >> addr;
	
	    proactor_engine engine;
	    bool rc = engine.add_acceptor("echo_server", addr, [&engine](bool status, connection *conn)->bool{
	        if (status)
	        {
                conn->set_recv_timeout(0);

                conn->register_recv_callback([](const int8 *data, shine::size_t len, connection *conn)->bool{
                    conn->async_send(data, len);
                    return true;
                });

                conn->async_recv();
            }
	
	        return true;
	    });
	
	    if (rc)
	    {
	        std::cout << "bind " << addr << "success." << endl;
	        engine.run();
	    }
	    else
	    {
	        std::cout << "bind " << addr << "failed." << endl;
	    }
	
	    return 0;
	}


echo客户端示例：

	#include <iostream>
	#include "../proactor_engine.hpp"
	
	using namespace shine;
	using namespace shine::net;
	
	int echo_client_main(){
	    std::cout << "connect address: ";
	    shine::string addr;
	    std::cin >> addr;
	
	    proactor_engine engine;
	
	    bool rc = engine.add_connector("client", addr, [](bool ok, connector *conn){
	        std::cout << "connect:" << ok << endl;
	        if (ok)
	        {
	            conn->set_recv_timeout(10000);
	
	            conn->register_recv_callback([](const int8 *data, shine::size_t len, connection *conn)->bool{
	                std::cout << "recv_callback len:" << len << " data:" << data << std::endl;
	                conn->get_timer_manager()->set_timer(5000, [conn]()->bool{
	                    shine::string str = "hello world!";
	                    conn->async_send(str.data(), str.size());                 
	                    return false;
	                });
	                return true;
	            });
	
	            conn->register_send_callback([](shine::size_t len, connection *conn)->bool{
	                return true;
	            });
	
	            conn->register_recv_timeout_callback([](connection *conn)->bool{
	                std::cout << "recv_timeout_callback:" << endl;
	                return true;
	            });
	
	            conn->register_send_timeout_callback([](connection *conn)->bool{
	                std::cout << "send_timeout_callback:" << endl;
	                return false;
	            });
	
	            conn->register_close_callback([](connection *conn){
	                std::cout << "close_callback:" << endl;
	            });
	
	            shine::string data = "hello world";
	            conn->async_send(data.data(), data.size());
	            conn->async_recv();
	        }
	    });
	
	    if (rc)
	    {
	        std::cout << "connect " << addr << "success." << endl;
	        engine.run();
	    }
	    else
	    {
	        std::cout << "connect " << addr << "failed." << endl;
	    }
	
	    return 0;
	}


### http http服务端/客户端封装 ###

http_base_server示例：

	#include <iostream>
	#include "../http_server.hpp"
	
	using namespace shine;
	using namespace shine::net;
	using namespace http;
	
	int base_server_main(){
	    proactor_engine engine;
	
	    shine::string addr = "0.0.0.0:8300";
	    bool rc = engine.add_acceptor("http_base_server", addr, [&engine](bool status, connection *conn)->bool{
	        if (status)
	        {
	            http::server_peer *conn_handle = new http::server_peer;
	
	                conn_handle->set_recv_timeout(15000);
	                conn_handle->register_url_handle("/", [](const http::request &request, http::response &response)->bool{
	                    response.set_version(request.get_version());
	                    response.set_status_code(200);
	                    response.set_body("hello shineframe!");
	                    return true;
	                });
	
	                conn_handle->register_url_handle("/api/*", [](const http::request &request, http::response &response)->bool{
	                    response.set_version(request.get_version());
	                    response.set_status_code(200);
	
	                    shine::string body = "hello api!\r\n\r\nparameters:\r\n";
	                    for (auto pa : request.get_url_parameters())
	                    {
	                        body += pa.first + "=" + pa.second + "\r\n";
	                    }
	
	                    response.set_body(body);
	                    return true;
	                });
	
	                conn_handle->run(conn);
	         }
	
	        return true;
	    });
	
	    if (rc)
	    {
	        std::cout << "bind " << addr << "success." << endl;
	        engine.run();
	    }
	    else
	    {
	        std::cout << "bind " << addr << "failed." << endl;
	    }
	
	    return 0;
	}


http_base_client抓取页面示例：

	#include <iostream>
	#include "../http_client.hpp"
	
	using namespace shine;
	using namespace shine::net;
	using namespace http;
	
	int base_client_main(){
	    sync_client clinet;
	    clinet.set_recv_timeout(3000);
	    clinet.get_request().set_host("www.baidu.com");
	    clinet.get_request().set_method(http::method::get);
	    clinet.get_request().set_url("/");
	
	    if (clinet.call())
	    {
	        shine::string tmp;
	        clinet.get_response().encode(tmp);
	        std::cout << tmp << std::endl;
	    }
	    else
	    {
	        std::cout << "" << std::endl;
	    }
	
	    return 0;
	}

### mysql封装 ###
封装了mysql c api，提供常用的数据库访问接口。

	#include <iostream>
	#include "db/mysql.hpp"
	
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

### websocket服务端封装 ###

echo服务端代码示例：

	#include <iostream>
	#include "websocket/websocket_server.hpp"

	using namespace shine;
	using namespace shine::net;
	using namespace websocket;
	
	int main(){
	    proactor_engine engine;
	
	    shine::string addr = "0.0.0.0:8300";
	    bool rc = engine.add_acceptor("websocket_echo_server", addr, [&engine](bool status, connection *conn)->bool{
	        if (status)
	        {
	            websocket::server_peer *conn_handle = new websocket::server_peer;
	            conn_handle->set_recv_timeout(0);
	
	            conn_handle->register_websocket_recv_callback([](frame_type type, const int8 *data, shine::size_t len, net::connection *conn)->bool{
	                
	                if (type == websocket::e_text)
	                {
	                    shine::string response = parser::encode(type, data, len);
	                    conn->async_send(response.data(), response.size());
	                }
	                else if (type == websocket::e_binary)
	                {
	                    shine::string response = parser::encode(type, data, len);
	                    conn->async_send(response.data(), response.size());
	                }
	
	                return true;            
	            });
	
	            conn_handle->register_websocket_close_callback([](net::connection *conn){
	                std::cout << "websocket_close_callback:" << conn << std::endl;
	            });
	
	            conn_handle->run(conn);
	         }
	
	        return true;
	    });
	
	    if (rc)
	    {
	        std::cout << "bind " << addr << "success." << endl;
	        engine.run();
	    }
	    else
	    {
	        std::cout << "bind " << addr << "failed." << endl;
	    }
	
	    return 0;
	}

