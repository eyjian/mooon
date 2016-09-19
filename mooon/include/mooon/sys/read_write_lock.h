#ifndef MOOON_SYS_READ_WRITE_LOCK
#define MOOON_SYS_READ_WRITE_LOCK
#include "mooon/sys/utils.h"
#include <pthread.h>
SYS_NAMESPACE_BEGIN

// 命令行工具debuginfo-install用于安装debug信息，示例（为glibc安装debug信息，以方便跟踪调试）：debuginfo-install glibc

/***
  * 读写锁类
  * 请注意: 谁加锁谁解锁原则，即一个线程加的锁，不能由另一线程来解锁
  * 而且加锁和解锁必须成对调用，否则会造成死锁
  */
class CReadWriteLock
{
public:
	CReadWriteLock() throw (CSyscallException);
	~CReadWriteLock() throw ();

    /***
      * 释放读或写锁
      * @exception: 出错抛出CSyscallException异常
      */
	void unlock() throw (CSyscallException);

    /***
      * 获取读锁，如果写锁正被持有则一直等待直到可获取到读锁
      * 注意同一个线程加了同一个对象的读锁后再加写锁会死锁！！！
      * @exception: 出错抛出CSyscallException异常
      */
	void lock_read() throw (CSyscallException);

    /***
      * 获取写锁，如果写锁正被持有则一直等待直到可获取到写锁
      * 注意同一个线程加了同一个对象的读锁后再加写锁会死锁！！！
      * @exception: 出错抛出CSyscallException异常
      */
	void lock_write() throw (CSyscallException);

    /***
      * 尝试去获取读锁，如果写锁正被持有则立即返回
      * @return: 如果成功获取了读锁，则返回true，否则返回false
      * @exception: 出错抛出CSyscallException异常
      */
	bool try_lock_read() throw (CSyscallException);

    /***
      * 尝试去获取写锁，如果写锁正被持有则立即返回
      * @return: 如果成功获取了写锁，则返回true，否则返回false
      * @exception: 出错抛出CSyscallException异常
      */
	bool try_lock_write() throw (CSyscallException);

    /***
      * 以超时方式获取读锁，如果写锁正被持有则等待指定的毫秒数，
      * 如果在指定的毫秒时间内，仍不能得到读锁，则立即返回
      * @millisecond: 等待获取读锁的毫秒数
      * @return: 如果在指定的毫秒时间内获取到了读锁，则返回true，否则如果超时则返回false
      * @exception: 出错抛出CSyscallException异常
      */
	bool timed_lock_read(uint32_t millisecond) throw (CSyscallException);

    /***
      * 以超时方式获取写锁，如果写锁正被持有则等待指定的毫秒数，
      * 如果在指定的毫秒时间内，仍不能得到写锁，则立即返回
      * @millisecond: 等待获取写锁的毫秒数
      * @return: 如果在指定的毫秒时间内获取到了写锁，则返回true，否则如果超时则返回false
      * @exception: 出错抛出CSyscallException异常
      */
	bool timed_lock_write(uint32_t millisecond) throw (CSyscallException);
	
private:
	pthread_rwlock_t _rwlock;
};

/***
  * 读锁帮助类，用于自动释放读锁
  */
class ReadLockHelper
{
public:
    ReadLockHelper(CReadWriteLock& lock)
      :_read_lock(lock)
    {
        _read_lock.lock_read();
    }    
    
    /** 析构函数，会自动调用unlock解锁 */
    ~ReadLockHelper()
    {
        _read_lock.unlock();
    }
    
private:
    CReadWriteLock& _read_lock;
};

/***
  * 读锁帮助类，用于自动释放写锁
  */
class WriteLockHelper
{
public:
    WriteLockHelper(CReadWriteLock& lock)
      :_write_lock(lock)
    {
        _write_lock.lock_write();
    }
    
    /** 析构函数，会自动调用unlock解锁 */
    ~WriteLockHelper()
    {
        _write_lock.unlock();
    }
    
private:
    CReadWriteLock& _write_lock;
};

SYS_NAMESPACE_END
#endif // MOOON_SYS_READ_WRITE_LOCK
