 /**
 *****************************************************************************
 *
 *@file fs_linux.hpp
 *
 *@brief linux文件系统封装,此文件完全参考复用了k子的代码
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
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/file.h>
#include <pwd.h>
#include <grp.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>

#include "../util/string.hpp"

namespace shine
{
    namespace filesystem
    {
            inline bool chdir(const string &path){
                return !::chdir(path.c_str());
            }

            inline string getcwd(){
                char buffer[4096];
                if (!::getcwd(buffer, sizeof(buffer)))
                    return{};
                string result(buffer);
                if (result.size() && (result.back() != '\\' || result.back() != '/'))
                    result.push_back('/');
                return std::move(result);
            }

			inline bool mkdir(const string &path, bool p = true, int mode = 0777){
                if (!p)
                    return !::mkdir(path.c_str(), mode);
                std::size_t offset = 0;
                do {

                    auto pos = path.find_first_of('\\', offset);
                    if (pos == string::npos)
                        pos = path.find_first_of('/', offset);
                    if (pos == string::npos)
                    {
                        return !::mkdir(path.c_str(), mode);
                    }
                    else {
                        auto parent_dir = path.substr(0, pos);
                        offset = pos + 1;
                        if (!::mkdir(parent_dir.c_str(), mode))
                            continue;
                        return false;
                    }
                } while (true);
            }

			inline bool rmdir(const string &path){
                return !::rmdir(path.c_str());
            }

            inline bool isdir(const string &path){
                struct stat st;
                if (::stat(path.c_str(), &st) != 0)
                    return false;
                return !!S_ISDIR(st.st_mode);
            }

            inline bool unlink(const string &path){
                return !::unlink(path.c_str());
            }

            inline bool file_exists(const string &path){
                return !::access(path.c_str(), F_OK);
            }

            inline string realpath(const string &path){
                char *real = ::realpath(path.c_str(), 0);
                if (real == nullptr)
                    return{};
                string result(real);
                free(real);
                return result;
            }

            inline bool rename(const string &old_name, const string &new_name){
                return !::rename(old_name.c_str(), new_name.c_str());
            }

            inline bool chmod(const string &path, int mode){
                return !::chmod(path.c_str(), mode);
            }

            inline bool chown(const string &path, const string &user){
                struct passwd *pwd = getpwnam(user.c_str());
                if (pwd == 0)
                    return false;
                return !::chown(path.c_str(), pwd->pw_uid, -1);
            }

            inline bool is_file(const string &path){
                struct stat st;
                if (::stat(path.c_str(), &st) != 0)
                    return false;
                return !!S_ISREG(st.st_mode);
            }

            inline bool islink(const string &path){
                struct stat st;
                if (stat(path.c_str(), &st) != 0)
                    return false;
                return !!S_ISLNK(st.st_mode);
            }

            inline bool is_readable(const string &path){
                return !::access(path.c_str(), R_OK);;
            }

            inline bool is_writable(const string &path){
                return !::access(path.c_str(), W_OK);
            }

            inline bool is_executable(const string &path){
                return !::access(path.c_str(), X_OK);
            }

            enum type
            {
                e_file,
                e_dir,
                e_link,
                e_block,
                e_socket,
                e_fifo,
                e_unknown,
            };

			inline type file_type(const string &path){
                struct stat st;
                if (stat(path.c_str(), &st) != 0)
                    return e_unknown;

                if (S_ISREG(st.st_mode))
                    return e_file;
                else if (S_ISDIR(st.st_mode))
                    return e_dir;
                else if (S_ISLNK(st.st_mode))
                    return e_link;
                else if (S_ISBLK(st.st_mode))
                    return e_block;
                else if (S_ISSOCK(st.st_mode))
                    return e_socket;
                else if (S_ISFIFO(st.st_mode))
                    return e_fifo;

                return e_unknown;
            }

            inline std::vector<string> ls_files(const string &path, std::size_t depth = 1){
                std::vector<string> files;
                DIR *pDir = ::opendir(path.c_str());
                struct dirent *ent;
                while ((ent = readdir(pDir)) != NULL)
                {
                    string name(ent->d_name);
                    string realpath;
                    if (path.back() == '\\' || path.back() == '/')
                        realpath = path + name;
                    else
                        realpath = path + "/" + path;

                    if (ent->d_type & DT_DIR)
                    {
                        if (name == "." || name == ".." || 1 == depth)
                            continue;
                        auto tmp = ls_files(realpath + "/", depth - 1);
                        for (auto &iter : tmp)
                            files.emplace_back(std::move(iter));
                    }
                    else if (ent->d_type & DT_REG)
                    {
                        files.emplace_back(std::move(name));
                    }
                }
                return std::move(files);
            }

            inline std::vector<string> ls(const string &path){
                std::vector<string> files;
                DIR *pDir = ::opendir(path.c_str());
                struct dirent *ent;
                while ((ent = readdir(pDir)) != NULL)
                {
                    string name(ent->d_name);
                    if (ent->d_type & DT_DIR)
                    {
                        if (!(name == "." || name == ".."))
                            files.emplace_back(path + name);
                    }
                    else if (ent->d_type & DT_REG)
                    {
                        files.emplace_back(path + name);
                    }
                }
                return std::move(files);
            }

            inline size_t last_modified(const string &filepath){
                struct stat st;
                if (stat(filepath.c_str(), &st) != 0)
                    return -1;
                return (size_t)st.st_mtime;
            }

            inline size_t file_size(const string &filepath){
                struct stat st;
                if (stat(filepath.c_str(), &st) != 0)
                    return -1;
                return (size_t)st.st_size;
            }

            inline string getenv(const string &name){
                char *env = ::getenv(name.c_str());
                if (env == nullptr)
                    return{};
                return string(env);
            }

            inline bool setenv(const string &name, const string &value){
                return !::setenv(name.c_str(), value.c_str(), 1);
            }

            inline void *mmap(const string &path){
                struct stat st;
                void *map;
                int fd = ::open(path.c_str(), O_RDONLY);
                if (fd < 0)
                {
                    return nullptr;
                }
                fstat(fd, &st);
                map = ::mmap(0, st.st_size, PROT_READ, MAP_PRIVATE | MAP_FILE, fd, 0);
                if (map == MAP_FAILED)
                    return nullptr;

                close(fd);
                return map;
            }

            inline string tempdir(){
                static const char *adirs[] =
                {
                    "/var/tmp",
                    "/usr/tmp",
                    "/usr/local/tmp"
                };
                unsigned int i;
                struct stat buf;
                const char *dir;
                dir = ::getenv("TMPDIR");
                if (dir && dir[0] != 0 && !access(dir, 07))
                    return string(dir);

                for (i = 0; i < sizeof(adirs) / sizeof(adirs[0]); i++)
                {
                    dir = adirs[i];
                    if (dir == 0)
                        continue;
                    if (stat(dir, &buf))
                        continue;
                    if (!S_ISDIR(buf.st_mode))
                        continue;
                    if (access(dir, 07))
                        continue;
                    return string(dir);
                }
                return string("/tmp");
            }

            inline uint32 process_id(){
                return (uint32)::getpid();
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
                file(const file &) = delete;
                file &operator= (const file &) = delete;

                file(file &&other)
                {
                    fd_ = other.fd_;
                    other.fd_ = -1;
                }
                file &operator= (file &&other)
                {
                    if (this != &other)
                    {
                        fd_ = other.fd_;
                        other.fd_ = -1;
                    }
                    return *this;
                }

                operator bool()
                {
                    return fd_ > 0;
                }
                bool operator !()
                {
                    return fd_ < 0;
                }
                int open(const char *zPath, int _open_mode)
                {
                    int mode = O_RDONLY;
                    if (_open_mode & OPEN_CREATE)
                    {
                        mode = O_CREAT;
                        if (_open_mode & OPEN_TRUNC)
                        {
                            mode |= O_TRUNC;
                        }
                    }
                    else if (_open_mode & OPEN_EXCL)
                    {
                        mode = O_CREAT | O_EXCL;
                    }
                    else if (_open_mode & OPEN_TRUNC)
                    {
                        mode = O_RDWR | O_TRUNC;
                    }
                    if (_open_mode & OPEN_RDWR)
                    {
                        mode &= ~O_RDONLY;
                        mode |= O_RDWR;
                    }
                    else if (_open_mode & OPEN_WRONLY)
                    {
                        mode &= ~O_RDONLY;
                        mode |= O_WRONLY;
                    }
                    if (_open_mode & OPEN_APPEND)
                    {
                        /* Append mode */
                        mode |= O_APPEND;
                    }
#ifdef O_TEMP
                    if (_open_mode & OPEN_TEMP)
                    {
                        mode |= O_TEMP;
                    }
#endif
#define JX9_UNIX_OPEN_MODE	0640
                    fd_ = ::open(zPath, mode, JX9_UNIX_OPEN_MODE);
                    if (fd_ < 0)
                        return false;
                    return true;
                }
                size_t read(void *pBuffer, size_t nDatatoRead)
                {
                    ssize_t bytes;
                    bytes = ::read(fd_, pBuffer, (size_t)nDatatoRead);
                    if (bytes < 1)
                    {
                        return -1;
                    }
                    return (size_t)bytes;
                }
                size_t write(const void *pBuffer, size_t len)
                {
                    const char *data = (const char *)pBuffer;
                    size_t count = 0;
                    ssize_t bytes = 0;
                    while (len > 0)
                    {
                        bytes = ::write(fd_, data, (size_t)len);
                        if (bytes < 1)
                        {
                            break;
                        }
                        len -= bytes;
                        count += bytes;
                        data += bytes;
                    }
                    if (len > 0)
                        return -1;
                    return count;
                }
                bool seek(size_t offset, whence _whence)
                {
                    int mode;
                    switch (_whence)
                    {
                    case whence::CURRENT:
                        mode = SEEK_CUR;
                        break;
                    case whence::END:
                        mode = SEEK_END;
                        break;
                    case 0:
                    default:
                        mode = SEEK_SET;
                        break;
                    }
                    return !!lseek(fd_, (off_t)offset, mode);
                }
                bool unlock()
                {
                    return !flock(fd_, LOCK_UN);
                }
                int lock(bool exclusive = true)
                {
                    if (exclusive)
                        return !flock(fd_, LOCK_EX);
                    return !flock(fd_, LOCK_SH);
                }
                size_t tell()
                {
                    return (size_t)lseek(fd_, 0, SEEK_CUR);
                }
                bool trunc(size_t nOfft)
                {
                    return !ftruncate(fd_, (off_t)nOfft);
                }
                bool sync()
                {
                    return !fsync(fd_);
                }
                void close()
                {
                    if (fd_ > 0)
                        ::close(fd_);
                    fd_ = -1;
                }
            private:
                int fd_ = -1;
            };
    }
}
