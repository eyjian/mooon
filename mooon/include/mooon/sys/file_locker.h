/**
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: jian yi, eyjian@qq.com
 */
#ifndef MOOON_SYS_FILE_LOCKER_H
#define MOOON_SYS_FILE_LOCKER_H
#include "mooon/sys/config.h"
#include <errno.h>
#include <stdio.h>
#include <string>
#include <sys/file.h>
#include <unistd.h>
SYS_NAMESPACE_BEGIN

// FileLocker 普通文件锁，总是锁定整个文件
// ExFileLocker 增强型文件锁，可只锁定文件指定部分
// ExclusiveFileLocker 独占普通文件锁
// SharedFileLocker 共享普通文件锁

/**
 * #include <sys/file.h>
 * 普通文件锁，总是锁定整个文件，支持独占和共享两种类型，由参数exclusive决定
 *
 * 进程退出时，它所持有的锁会被自动释放，也可用于同一个进程内的多线程互斥
 * 但请注意：同一个FileLocker对象，不要跨线程使用，同一个FileLocker对象总是只会被同一个线程调度
 */
class FileLocker
{
public:
    /***
     * 不加锁构造函数，由调用者决定何时加锁
     */
    explicit FileLocker(const char* filepath) throw ()
        : _fd(-1), _filepath(filepath)
    {
    }

    /***
     * 自动加锁构造函数
     * ，建议使用SharedFileLocker或ExclusiveFileLocker，替代此构造函数调用
     * @exclusive 是否独占锁
     */
    explicit FileLocker(const char* filepath, bool exclusive) throw ()
        : _fd(-1), _filepath(filepath)
    {
        const bool nonblocking = false;
        lock(exclusive, nonblocking);
    }

    ~FileLocker() throw ()
    {
        unlock();
    }

    /**
     * 加锁
     * @exclusive 是否独占锁
     */
    bool lock(bool exclusive, bool nonblocking) throw ()
    {
        // 独占还是共享
        int operation = exclusive? LOCK_EX: LOCK_SH;
        if (nonblocking)
            operation |= LOCK_NB;

        _fd = open(_filepath.c_str(), O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        if (_fd != -1)
        {
            if (-1 == flock(_fd, operation))
            {
                int errcode = errno; // Keep
                close(_fd); // 无write动作，不用判断close()的返回值

                _fd = -1;
                errno = errcode; // 恢复，目的是让上层调用者可以使用errno
            }
        }

        return _fd != -1;
    }

    /**
     *  FileLocker file_locker(filepath);
     *  if (!file_locker.try_lock() && (file_locker.would_block())
     *  {
     *      // 尝试加锁不成功，原因是其它线程或进程已独占该锁
     *  }
     */
    bool would_block() const throw()
    {
        return EWOULDBLOCK == errno;
    }

    bool try_lock(bool exclusive) throw()
    {
        return lock(exclusive, true);
    }

    bool unlock() throw ()
    {
        bool ret = false;
    
        if (is_locked())
        {
            if (0 == flock(_fd, LOCK_UN))
            {
                ret = true;
                close(_fd);
                _fd = -1;
            }
        }
        
        return ret;
    }

    bool is_locked() const throw ()
    {
        return _fd != -1;
    }

    operator bool () const
    {
        return _fd != -1;
    }

    /***
     * 返回锁文件路径
     */
    const std::string& get_filepath() const throw ()
    {
        return _filepath;
    }

private:
    int _fd;
    std::string _filepath;
};

/**
 * 普通独占文件锁
 */
class ExclusiveFileLocker: public FileLocker
{
public:
    explicit ExclusiveFileLocker(const char* filepath)
        : FileLocker(filepath, true)
    {
    }

private:
    bool lock();
    bool unlock();
};

/**
 * 普通共享文件锁
 */
class SharedFileLocker: public FileLocker
{
public:
    explicit SharedFileLocker(const char* filepath)
        : FileLocker(filepath, false)
    {
    }

private:
    bool lock();
    bool unlock();
};

/**
 * #include <unistd.h>
 * 增强型文件锁，可只锁文件指定的部分
 * ，一个ExFileLocker对象，总是只被一个线程调用
 */
class ExFileLocker
{
public:
    /***
     * 构造增强型文件锁对象
     * ，调用者需要负责文件的打开和关闭
     * @fd 已打开的文件描述符（即文件句柄）
     *     ，fd必须以O_WRONLY或O_RDWR方式打开，否则遇到EBADF错误
     *     ，但如果调用进程具有PRIV_LOCKRDONLY权限的组的成员，则可使用O_RDONLY打开fd
     */
    explicit ExFileLocker(int fd)
        : _fd(fd)
    {
    }

    /***
     * 加锁
     * ，如果抢到锁，则立即返回true，否则被阻塞
     * ，如果加锁失败，则返回false
     * @size 如果为0，表示锁定从文件当前偏移开始到文件尾的区域
     *       ，如果为正，则表示锁定从文件当前偏移开始，往后大小为size的连续区域
     *       ，如果为负，则表示锁定从文件当前偏移开始，往前大小为size的连续区域
     * 失败原因可以通过errno取得
     */
    bool lock(off_t size)
    {
        if (_fd != -1)
        {
            return 0 == lockf(_fd, F_LOCK, size);
        }

        return false;
    }

    /**
     * 尝试加锁，总是立即返回，不会被阻塞
     * ，如果加锁成功，则立即返回true，否则立即返回false
     * @size 如果为0，表示尝试锁定从文件当前偏移开始到文件尾的区域
     *       ，如果为正，则表示尝试锁定从文件当前偏移开始，往后大小为size的连续区域
     *       ，如果为负，则表示尝试锁定从文件当前偏移开始，往前大小为size的连续区域
     * 失败原因可以通过errno取得
     */
    bool try_lock(off_t size)
    {
        if (_fd != -1)
        {
            return 0 == lockf(_fd, F_TLOCK, size);
        }

        return false;
    }

    /***
     * 检测在指定的区域中是否存在其他进程的锁定
     * ，如果该区域可访问，则返回true，否则返回false
     * @size 如果为0，表示检测从文件当前偏移开始到文件尾的区域的锁定状态
     *        ，如果为正，则表示检测从文件当前偏移开始，往后大小为size的连续区域的锁定状态
     *        ，如果为负，则表示检测从文件当前偏移开始，往前大小为size的连续区域的锁定状态
     * 失败原因可以通过errno取得
     */
    bool test_lock(off_t size)
    {
        if (_fd != -1)
        {
            return 0 == lockf(_fd, F_TEST, 0);
        }

        return false;
    }

    /**
     * 如果解锁成功，则立即返回true，否则立即返回false
     * @size 如果为0，表示解锁从文件当前偏移开始到文件尾的区域
     *       ，如果为正，则表示解锁从文件当前偏移开始，往后大小为size的连续区域
     *       ，如果为负，则表示解锁从文件当前偏移开始，往前大小为size的连续区域
     * 失败原因可以通过errno取得
     */
    bool unlock(off_t size)
    {
        if (_fd != -1)
        {
            return 0 == lockf(_fd, F_ULOCK, size);
        }

        return false;
    }

private:
    int _fd;
};

/**
 * 使用示例：
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// 启动两个进程，并保持argv[1]相同，即可观察互斥效果
int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "usage: %s lock_filepath1 lock_filepath2\n", argv[0]);
        exit(1);
    }
    
    bool locked = false;
    fprintf(stdout, "=========ExclusiveFileLocker=========\n");
    {
        ExclusiveFileLocker file_locker(argv[1]);
        locked = file_locker.is_locked();

        if (!locked)
        {
            fprintf(stderr, "failed to lock %s: %s\n", argv[1], strerror(errno));
        }
        else
        {
            fprintf(stdout, "locked %s successfully\n", argv[1]);
        }
        
        fprintf(stdout, "press ENTER to unlock %s\n", argv[1]);
        getchar();
    }
    if (locked)
    {
        fprintf(stdout, "unlocked %s\n", argv[1]);
    }
    
    fprintf(stdout, "\n=========ExFileLocker=========\n");
    int fd = open(argv[2], O_WRONLY|O_APPEND, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (-1 == fd)
    {
        fprintf(stderr, "failed to open %s: %s\n", argv[2], strerror(errno));
        exit(1);
    }
    else
    {
        ExFileLocker ex_file_locker(fd);

        if (!ex_file_locker.lock(0))
        {
            fprintf(stderr, "failed to lock %s: %s\n", argv[2], strerror(errno));
        }
        else
        {
            fprintf(stdout, "locked %s successfully\n", argv[2]);

            if (ex_file_locker.test_lock(0))
            {
                fprintf(stdout, "%s locked\n", argv[2]);
            }
            else
            {
                fprintf(stdout, "%s not locked\n", argv[2]);
            }

            fprintf(stdout, "press ENTER to unlock %s\n", argv[2]);
            getchar();

            if (ex_file_locker.unlock(0))
            {
                fprintf(stdout, "unlock %s successfully\n", argv[2]);
            }
            else
            {
                fprintf(stdout, "failed to unlock %s: %s\n", argv[2], strerror(errno));
            }
        }

        close(fd);
    }

    return 0;
}
*/

SYS_NAMESPACE_END
#endif // MOOON_SYS_FILE_LOCKER_H
