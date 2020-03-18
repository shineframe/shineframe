 /**
 *****************************************************************************
 *
 *@file file.hpp
 *
 *@brief 文件操作基本封装
 *
 *@todo 
 * 
 *@note shineframe开发框架 https://github.com/shineframe/shineframe
 *
 *@author sunjian 39215174@qq.com
 *
 *@version 1.0
 *
 *@date 2018/6/15 
 *****************************************************************************
 */
#pragma once

#include <iostream>
#include "../common/define.hpp"
#include "filesystem.hpp"

#if (defined SHINE_OS_WINDOWS)
#include <io.h>
#define FSEEK _fseeki64
#define FTELL _ftelli64
#else
#include <stdio.h>
#define FSEEK fseeko64
#define FTELL ftello64
#endif


using namespace std;

namespace shine
{
    class file{
    public:
        file(){

        }

        ~file(){
            close();
        }

#ifdef SHINE_OS_WINDOWS
    private:
        FILE * fopen(const int8 *path, const int8 *mode){
            FILE *ret = 0;
            fopen_s(&ret, path, mode);
            
            return ret;
        }
    public:
#endif

        bool open(const string &path, bool create = true, bool readonly = false){

            if ((create && !readonly) && !filesystem::file_exists(path))
            {
                size_t pos = path.rfind('\\');
                if (pos == string::npos)
                    pos = path.rfind('/');
              
                if (pos != string::npos)
                    filesystem::mkdir(path.substr(0, pos));

                FILE *tmp = fopen(path.c_str(), "wb");
                if (tmp != 0)
                    fclose(tmp);
            }

            _file = fopen(path.c_str(), readonly ? "rb" : "rb+");
            _path = path;
            return _file != 0;
        }

        void close(){
            _path.clear();
            if (_file == 0) return;

            fclose(_file);
            _file = 0;
        }

        bool seek(size_t offset, int32 from_where){
            if (_file == 0) return false;

            return FSEEK(_file, offset, from_where) == 0;
        }

        size_t tell(){
            if (_file == 0) return 0;

            int64 ret =  FTELL(_file);
            return ret < 0 ? 0 : (size_t)ret;
        }

        size_t size(){
            if (_file == 0) return 0;

            size_t cur = FTELL(_file);
            FSEEK(_file, 0, SEEK_END);

            size_t ret = FTELL(_file);
            if (ret != cur)
                FSEEK(_file, cur, SEEK_SET);

            return ret;
        }

        bool resize(size_t size){
            if (_file == 0) return false;
#if (defined SHINE_OS_WINDOWS) 
            return _chsize_s(_fileno(_file), size) == 0;
#else
            return ftruncate(fileno(_file), size) == 0;
#endif
        }

        size_t read(int8 *buf, size_t size){
            if (_file == 0) return 0;

            return fread(buf, 1, size, _file);
        }

        size_t write(const int8 *buf, size_t size){
            if (_file == 0) return 0;

            auto rc = fwrite(buf, 1, size, _file);
            return rc;
        }

        size_t append(const std::string &data){
            return append(data.data(), data.size());
        }

        size_t append(const int8 *buf, size_t size){
            if (seek(0, SEEK_END))
                return write(buf, size);
            return 0;
        }

        bool save(){
            if (_file == 0) return false;

            fflush(_file);
            return true;
        }

        bool save(const string &buf){
            resize(buf.size());
            seek(0, SEEK_SET);

            auto rc = write(buf.data(), buf.size());
            if (rc != buf.size())
                return false;

            return save();
        }

        void clear(){
            resize(0);
        }

        size_t readall(string &buf){
            if (_file == 0) return 0;

            buf.resize(size());

            size_t cur_pos = tell();
            seek(0, SEEK_SET);
            size_t ret = fread((void*)buf.data(), 1, buf.size(), _file);

            if (ret != cur_pos)
                seek(cur_pos, SEEK_SET);

            return ret;
        }

        const string &path() const{
            return _path;
        }
    private:
        FILE *_file = 0;
        string _path;
    };
}