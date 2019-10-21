 /**
 *****************************************************************************
 *
 *@file fs_windows.hpp
 *
 *@brief windows文件系统封装,此文件完全参考复用了k子的代码
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
#include "../net/socket.hpp"

using namespace std;

#include <windows.h>
#include <string>
#include <vector>

namespace shine
{
    namespace filesystem
    {
        inline string username() {
            string ret;
            ret.resize(1024);
            DWORD len = (DWORD)ret.size();
            if (!GetUserName((LPSTR)ret.data(), &len))
                ret.resize(0);
            else
                ret.resize(len);
            return std::move(ret);
        }

        inline size_t process_id() {
            return GetProcessId(GetCurrentProcess());
        }

        inline string tempdir() {
            string ret;
            ret.resize(1024);

            DWORD len = GetTempPath((DWORD)ret.size(), (LPSTR)ret.data());
            if (len < 1)
                return ret = "C:\\Windows\\Temp";
            else
                ret.resize(len);
            return std::move(ret);
        }

        inline HANDLE open_read_only(const string &path) {
            DWORD dwType = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS;
            DWORD dwShare = FILE_SHARE_READ | FILE_SHARE_WRITE;
            DWORD dwAccess = GENERIC_READ;
            DWORD dwCreate = OPEN_EXISTING;
            HANDLE pHandle;
            pHandle = CreateFile(path.c_str(), dwAccess, dwShare, 0, dwCreate, dwType, 0);
            if (pHandle == INVALID_HANDLE_VALUE)
                return nullptr;
            return pHandle;
        }

        inline bool chdir(const string &path) {
            return !!SetCurrentDirectory(path.c_str());
        }

        inline string getcwd() {
            char buf[2048] = { 0 };
            GetCurrentDirectory(sizeof(buf), buf);
            string tmp(buf);
            if (tmp.size() && (tmp.back() != '\\'))
                tmp.push_back('\\');

            string ret;
            for (auto &iter : tmp)
            {
                if (iter == '\\')
                    ret.push_back('/');
                else
                    ret.push_back(iter);
            }
            return std::move(ret);
        }

        inline bool mkdir(const string &path, bool p = true)
        {
            if (!p)
                return !!CreateDirectory(path.c_str(), 0);

            size_t offset = 0;
            do {
                size_t pos = path.find_first_of('\\', offset);
                if (pos == string::npos)
                    pos = path.find_first_of('/', offset);
                if (pos == string::npos)
                {
                    return !!CreateDirectory(path.c_str(), NULL) ||
                        ERROR_ALREADY_EXISTS == GetLastError();
                }
                else {
                    string parent_dir = path.substr(0, pos);
                    offset = pos + 1;
                    if (!!CreateDirectory(parent_dir.c_str(), NULL) ||
                        ERROR_ALREADY_EXISTS == GetLastError())
                        continue;
                    return false;
                }
            } while (true);
        }

        inline bool rmdir(const string &path) {
            return !!RemoveDirectory(path.c_str());
        }

        inline bool isdir(const string &path) {
            DWORD dwAttr;
            dwAttr = GetFileAttributes(path.c_str());
            if (dwAttr == INVALID_FILE_ATTRIBUTES)
                return false;
            return !!(dwAttr & FILE_ATTRIBUTE_DIRECTORY);
        }

        inline bool rename(const string &old, const string &_new) {
            return !!MoveFile(old.c_str(), _new.c_str());
        }

        inline string realpath(const string &path)
        {
            DWORD len;
            string ret;
            len = GetFullPathName(path.c_str(), 0, 0, 0);
            if (len > 0)
            {
                ret.resize(len);
                GetFullPathName(path.c_str(), len, (char*)ret.data(), 0);
            }
            return std::move(ret);
        }

        inline bool unlink(const string &path) {
            return !!DeleteFile(path.c_str());
        }

        inline size_t disk_free_space(const string &path) {
            DWORD dwSectPerClust, dwBytesPerSect, dwFreeClusters, dwTotalClusters;
            BOOL rc;
            string buffer;
            auto pos = path.find_first_of('\\');
            if (pos == string::npos)
                pos = path.find_first_of('/');

            if (pos == string::npos)
                buffer = path;
            else
                buffer = path.substr(0, pos);

            rc = GetDiskFreeSpace(buffer.c_str(), &dwSectPerClust, &dwBytesPerSect, &dwFreeClusters, &dwTotalClusters);
            if (!rc)
                return 0;
            return (size_t)dwFreeClusters * dwSectPerClust * dwBytesPerSect;
        }

        inline size_t disk_total_space(const string &path) {
            DWORD dwSectPerClust, dwBytesPerSect, dwFreeClusters, dwTotalClusters;
            string buffer;
            auto pos = path.find_first_of('\\');
            if (pos == string::npos)
                pos = path.find_first_of('/');

            if (pos == string::npos)
                buffer = path;
            else
                buffer = path.substr(0, pos);

            if (!GetDiskFreeSpace(buffer.c_str(), &dwSectPerClust, &dwBytesPerSect, &dwFreeClusters, &dwTotalClusters))
                return 0;
            return
                (size_t)dwTotalClusters * dwSectPerClust * dwBytesPerSect;
        }

        enum type
        {
            e_file,
            e_dir,
            e_link,
            e_block,
            e_unknown,
        };

        inline type file_type(const string &path) {
            DWORD dwAttr;
            dwAttr = GetFileAttributes(path.c_str());
            if (dwAttr == INVALID_FILE_ATTRIBUTES)
                return type::e_unknown;

            if (dwAttr & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE))
                return type::e_file;
            else if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
                return type::e_dir;
            else if (dwAttr & FILE_ATTRIBUTE_REPARSE_POINT)
                return type::e_link;
            else if (dwAttr & (FILE_ATTRIBUTE_DEVICE))
                return type::e_block;

            return type::e_unknown;
        }

        inline bool is_file(const string zPath)
        {
            DWORD dwAttr = GetFileAttributes(zPath.c_str());
            if (dwAttr == INVALID_FILE_ATTRIBUTES)
                return false;

            return !!(dwAttr & (FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_ARCHIVE));
        }

        inline std::vector<string> ls(const string &path) {
            std::vector<string> ret;
            WIN32_FIND_DATA find_data;
            HANDLE handle = ::FindFirstFile((path + "*.*").c_str(), &find_data);
            if (INVALID_HANDLE_VALUE == handle)
                return{};
            while (TRUE)
            {
                if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    if (strcmp(find_data.cFileName, ".") == 0 ||
                        strcmp(find_data.cFileName, "..") == 0)
                        ;
                    else
                    {
                        ret.emplace_back(path +find_data.cFileName);
                        //ret.back().push_back('/');
                    }
                }
                else if (find_data.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN |
                    FILE_ATTRIBUTE_NORMAL |
                    FILE_ATTRIBUTE_ARCHIVE))
                {
                    ret.emplace_back(path + find_data.cFileName);
                }
                if (!FindNextFile(handle, &find_data))
                    break;
            }
            FindClose(handle);
            return std::move(ret);
        }

        inline std::vector<string> ls_files(const string &path, size_t depth = 1) {
            std::vector<string> ret;
            WIN32_FIND_DATA find_data;
            HANDLE handle = ::FindFirstFile((path + "*.*").c_str(), &find_data);
            if (INVALID_HANDLE_VALUE == handle)
                return{};
            while (TRUE)
            {
                string filename(find_data.cFileName);
                string realpath;
                if (path.size() && (path.back() == '\\' || path.back() == '/'))
                    realpath = path + filename;
                else
                    realpath = path + "/" + filename;

                auto type = file_type(realpath);
                if (type == e_file)
                {
                    ret.emplace_back(realpath);
                }
                else if (type == e_dir)
                {
                    if (filename != "." && filename != ".." && depth > 1)
                    {
                        auto tmp = ls_files(realpath + "/", depth - 1);
                        for (auto &iter : tmp)
                            ret.emplace_back(std::move(iter));
                    }
                }
                if (!FindNextFile(handle, &find_data))
                    break;
            }
            FindClose(handle);
            return std::move(ret);
        }

        inline bool is_line(const string &path) {
            DWORD dwAttr = GetFileAttributes(path.c_str());
            if (dwAttr == INVALID_FILE_ATTRIBUTES)
                return false;

            return !!(dwAttr & FILE_ATTRIBUTE_REPARSE_POINT);
        }

        inline bool is_writable(const string &path) {
            DWORD dwAttr = GetFileAttributes(path.c_str());
            if (dwAttr == INVALID_FILE_ATTRIBUTES)
                return false;

            if ((dwAttr & (FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_NORMAL)) == 0)
                return false;

            if (dwAttr & FILE_ATTRIBUTE_READONLY)
                return false;

            return true;
        }

        inline bool is_executable(const string &zPath) {
            DWORD dwAttr = GetFileAttributes(zPath.c_str());
            if (dwAttr == INVALID_FILE_ATTRIBUTES)
                return false;

            if ((dwAttr & FILE_ATTRIBUTE_NORMAL) == 0)
                return false;

            return true;
        }

        inline size_t to_time(FILETIME pTime) {
            const size_t TICKS_PER_SECOND = 10000000;
            const size_t EPOCH_DIFFERENCE = 11644473600LL;
            size_t input, temp;
            input = pTime.dwHighDateTime;
            input <<= 32;
            input += pTime.dwLowDateTime;
            temp = input / TICKS_PER_SECOND;
            temp = temp - EPOCH_DIFFERENCE;
            return temp;
        }

        inline size_t last_modified(const string &filepath) {
            BY_HANDLE_FILE_INFORMATION sInfo;
            HANDLE pHandle = open_read_only(filepath);
            if (pHandle == 0)
                return -1;
            if (!GetFileInformationByHandle(pHandle, &sInfo))
                return -1;
            CloseHandle(pHandle);
            return to_time(sInfo.ftLastWriteTime);
        }

        inline size_t file_size(const string &filepath) {
            DWORD dwLow, dwHigh;
            HANDLE handle = open_read_only(filepath);
            if (!handle)
                return -1;
            dwLow = GetFileSize(handle, &dwHigh);
            CloseHandle(handle);
            size_t nSize = dwHigh;
            nSize <<= 32;
            nSize += dwLow;
            return nSize;
        }

        inline bool file_exists(const string &path) {
            DWORD dwAttr;
            dwAttr = GetFileAttributes(path.c_str());
            if (dwAttr == INVALID_FILE_ATTRIBUTES)
                return false;
            return true;
        }

        inline void * mmap(const string &path, size_t &size) {
            DWORD size_low, size_high;
            HANDLE file_handle, map_handle;
            void *map;

            file_handle = open_read_only(path);
            if (!file_handle)
                return nullptr;
            size_low = GetFileSize(file_handle, &size_high);
            map_handle = CreateFileMapping(file_handle,
                0, PAGE_READONLY, size_high, size_low, 0);
            if (map_handle == 0)
            {
                CloseHandle(file_handle);
                return nullptr;
            }
            size = ((size_t)size_high << 32) | size_low;
            map = MapViewOfFile(map_handle,
                FILE_MAP_READ, 0, 0, (SIZE_T)(size));
            CloseHandle(map_handle);
            CloseHandle(file_handle);
            return map;
        }

        inline void unmap(void *handle, size_t size = 0) {
            UnmapViewOfFile(handle);
        }

        inline string get_env(const string &name) {
            char buffer[1024];
            DWORD n = GetEnvironmentVariable(name.c_str(), buffer, sizeof(buffer));
            if (!n)
                return{};

            buffer[n] = '\0';
            return buffer;
        }

        inline bool set_env(const string &name, const string &value) {
            return !!SetEnvironmentVariable(name.c_str(), value.c_str());
        }

        class file
        {
        public:
            enum open_mode
            {
                OPEN_RDONLY = 0x001,
                OPEN_WRONLY = 0x002,
                OPEN_RDWR = 0x004,
                OPEN_CREATE = 0x008,
                OPEN_TRUNC = 0x010,
                OPEN_APPEND = 0x020,
                OPEN_EXCL = 0x040,
                OPEN_BINARY = 0x080,
                OPEN_TEMP = 0x100,
                OPEN_TEXT = 0x200
            };

            enum whence
            {
                BEGIN = 0,
                CURRENT = 1,
                END = 2
            };
            file()
            {

            }
            ~file()
            {
                close();
            }
            file(const file&) = delete;
            file &operator = (const file&) = delete;
            file(file &&other)
            {
                handle_ = other.handle_;
                other.handle_ = INVALID_HANDLE_VALUE;
            }
            file &operator = (file &&other)
            {
                if (this != &other)
                {
                    handle_ = other.handle_;
                    other.handle_ = INVALID_HANDLE_VALUE;
                }
                return *this;
            }
            operator bool()
            {
                return handle_ != INVALID_HANDLE_VALUE;
            }
            bool operator !()
            {
                return handle_ == INVALID_HANDLE_VALUE;
            }
            bool open(const std::string &filepath, int mode = open_mode::OPEN_CREATE)
            {
                DWORD type = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS;
                DWORD access = GENERIC_READ;
                DWORD share, create;

                if (mode & OPEN_CREATE)
                {
                    create = OPEN_ALWAYS;
                    if (mode & OPEN_TRUNC)
                        create = CREATE_ALWAYS;
                }
                else if (mode & OPEN_EXCL)
                {
                    create = CREATE_NEW;
                }
                else if (mode & OPEN_TRUNC)
                {
                    create = TRUNCATE_EXISTING;
                }
                else
                {
                    create = OPEN_EXISTING;
                }

                if (mode & OPEN_RDWR)
                {
                    access |= GENERIC_WRITE;
                }
                else if (mode & OPEN_WRONLY)
                {
                    access = GENERIC_WRITE;
                }
                if (mode & OPEN_APPEND)
                {
                    access = FILE_APPEND_DATA;
                }
                if (mode & OPEN_TEMP)
                {
                    type = FILE_ATTRIBUTE_TEMPORARY;
                }
                share = FILE_SHARE_READ | FILE_SHARE_WRITE;
                handle_ = CreateFile((LPCSTR)filepath.c_str(),
                    access,
                    share,
                    0,
                    create,
                    type,
                    0);
                return handle_ != INVALID_HANDLE_VALUE;
            }

            size_t read(void *buffer, size_t to_reads)
            {
                DWORD nRd;
                BOOL rc;
                rc = ReadFile(handle_, buffer, (DWORD)to_reads, &nRd, 0);
                if (!rc)
                {
                    std::cout << GetLastError() << std::endl;
                    return -1;
                }
                return (size_t)nRd;
            }
            size_t write(const void *buffer, size_t len)
            {
                const char *data = (const char *)buffer;
                size_t count = 0;
                DWORD bytes = 0;
                while (len > 0)
                {
                    if (!WriteFile(handle_, data, (DWORD)len, &bytes, 0))
                        break;
                    len -= bytes;
                    count += bytes;
                    data += bytes;
                }
                if (len > 0)
                {
                    return -1;
                }
                return count;
            }
            bool seek(size_t offset, whence _whence)
            {
                
                DWORD mode, ret;
                LONG high_offset;
                switch (_whence)
                {
                case whence::CURRENT:
                    mode = FILE_CURRENT;
                    break;
                case whence::END:
                    mode = FILE_END;
                    break;
                case 0:
                default:
                    mode = FILE_BEGIN;
                    break;
                }

                high_offset = (LONG)(offset >> 32);
                ret = SetFilePointer(handle_, (LONG)offset, &high_offset, mode);
                if (ret == INVALID_SET_FILE_POINTER)
                    return false;
                return true;
            }
            bool lock(bool exclusive = true)
            {
                DWORD low_bytes = 0, high_bytes = 0;
                OVERLAPPED dummy;
                memset(&dummy, 0, sizeof(dummy));

                DWORD flags = LOCKFILE_FAIL_IMMEDIATELY;
                if (exclusive == 1)
                    flags |= LOCKFILE_EXCLUSIVE_LOCK;
                low_bytes = GetFileSize(handle_, &high_bytes);
                return !!LockFileEx(handle_, flags, 0, low_bytes, high_bytes, &dummy);
            }
            bool unlock()
            {
                DWORD dwLo = 0, dwHi = 0;
                OVERLAPPED sDummy;
                memset(&sDummy, 0, sizeof(sDummy));
                return !!UnlockFileEx(handle_, 0, dwLo, dwHi, &sDummy);
            }
            size_t tell()
            {
                DWORD pos;
                pos = SetFilePointer(handle_, 0, 0, FILE_CURRENT);
                if (pos == INVALID_SET_FILE_POINTER)
                    return -1;
                return (size_t)pos;
            }
            bool trunc(size_t offset)
            {
                LONG high_offset;
                DWORD pos;
                high_offset = (LONG)(offset >> 32);
                pos = SetFilePointer(handle_, (LONG)offset, &high_offset, FILE_BEGIN);
                if (pos == INVALID_SET_FILE_POINTER)
                    return false;
                return !!SetEndOfFile(handle_);
            }
            bool sync()
            {
                return !!FlushFileBuffers(handle_);
            }
            void close()
            {
                if (handle_ != INVALID_HANDLE_VALUE)
                    CloseHandle(handle_);
                handle_ = INVALID_HANDLE_VALUE;
            }
        private:
            HANDLE handle_ = INVALID_HANDLE_VALUE;
        };
    }
}