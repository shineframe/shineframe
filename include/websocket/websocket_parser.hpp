#pragma once

#include <iostream>
#include <chrono>
#include <ctime>
#include <string>
#include <mutex>
#include <map>
#include <unordered_map>
#include <vector>
#include <functional>
#include "../common/define.hpp"
#include "../util/string.hpp"
#include "../http/http_parser.hpp"

#if (defined SHINE_OS_WINDOWS)
#else
#endif



// using namespace std;

namespace shine
{
    namespace websocket
    {
        enum frame_type{
            e_error = 0xFF00,
            e_incomplete = 0xFE00,
            e_opening = 0x3300,
            e_closing = 0x3400,
            e_incomplete_text = 0x01,
            e_incomplete_binary = 0x02,
            e_text = 0x81,
            e_binary = 0x82,
            e_ping = 0x19,
            e_pong = 0x1A
        };

        class parser{
        public:
            static string encode(frame_type type, const int8* data, size_t len)
            {
                string ret;

                ret += (uint8)type;

                if (len <= 125) {
                    ret += (int8)len;
                }
                else if (len <= 65535) {
                    ret += (int8)126; //16 bit length follows
                    ret += (int8)((len >> 8) & 0xFF); // leftmost first
                    ret += (int8)(len & 0xFF);
                }
                else { // >2^16-1 (65535)
                    ret += (int8)127;

                    for (int i = 3; i >= 0; i--) 
                        ret += (int8)0;

                    for (int i = 3; i >= 0; i--) 
                        ret += (int8)((len >> 8 * i) & 0xFF);
                }
                
                ret.append(data, len);
                return ret;
            }

            static frame_type decode(uint8* data, size_t len, uint8 *&out, size_t &out_len, size_t &cost_len)
            {
                //printf("getTextFrame()\n");
                if (len < 3) return e_incomplete;

                uint8 msg_opcode = data[0] & 0x0F;
                uint8 msg_fin = (data[0] >> 7) & 0x01;
                uint8 msg_masked = (data[1] >> 7) & 0x01;

                // *** message decoding 

                size_t payload_length = 0;
                int32 pos = 2;
                int32 length_field = data[1] & (~0x80);
                uint32 mask = 0;

                if (length_field <= 125) {
                    payload_length = length_field;
                }
                else if (length_field == 126) { //msglen is 16bit!
                    payload_length = ( (data[2] << 8) | (data[3]) );
                    pos += 2;
                }
                else if (length_field == 127) { //msglen is 64bit!
                    payload_length = (
                        ((size_t)data[2] << 56) |
                        ((size_t)data[3] << 48) |
                        ((size_t)data[4] << 40) |
                        ((size_t)data[5] << 32) |
                        ((size_t)data[6] << 24) |
                        ((size_t)data[7] << 16) |
                        ((size_t)data[8] << 8) |
                        ((size_t)data[9])
                        );
                    pos += 8;
                }

                //printf("PAYLOAD_LEN: %08x\n", payload_length);
                if (len < payload_length + pos)
                    return e_incomplete;             

                if (msg_masked) {
                    mask = *((unsigned int*)(data + pos));
                    pos += 4;

                    // unmask data:
                    uint8* ch = data + pos;
                    for (size_t i = 0; i<payload_length; i++) {
                        ch[i] = ch[i] ^ ((unsigned char*)(&mask))[i % 4];
                    }
                }

                out = data + pos;
                out_len = payload_length;
                cost_len = pos + payload_length;

                if (msg_opcode == 0x0) return (msg_fin) ? e_text : e_incomplete_text; // continuation frame ?
                if (msg_opcode == 0x1) return (msg_fin) ? e_text : e_incomplete_text;
                if (msg_opcode == 0x2) return (msg_fin) ? e_binary : e_incomplete_binary;
                if (msg_opcode == 0x9) return e_ping;
                if (msg_opcode == 0xA) return e_pong;

                return e_error;
            }
        };

        enum {
            e_decode_header = 0,
            e_decode_body = 1,
            e_decode_done = 2,
        };

    }
}
