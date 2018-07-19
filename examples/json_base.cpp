#include <iostream>
#include "util/json.hpp"

using namespace shine;

int main(){

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
