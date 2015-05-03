#include <assert.h>
#include <mooon/sys/utils.h>
#include <mooon/sys/pool_thread.h>
#include <mooon/sys/thread_pool.h>
#include <mooon/utils/exception.h>
MOOON_NAMESPACE_USE

// 测试线程，请注意是继承池线程CPoolThread，而不是线程CThread，
// 另外CPoolThread并不是CThread的子类
class CTestThread: public sys::CPoolThread
{
public:
	CTestThread()
		:_number_printed(0)
	{
	}
	
private:
	virtual void run()
	{
		// 在这个函数里实现需要池线程干的事
		if (_number_printed++ < 3)
		{
			// 只打印三次，以便观察效率
			printf("thread %u say hello.\n", get_thread_id());
		}

        // do_millisleep是由CPoolThread提供给子类睡眠用的，
        // 可以通过调用wakeup中断睡眠
		do_millisleep(1000);
	}
	
	virtual void before_start() throw (utils::CException, sys::CSyscallException)
	{
	}
	
private:
	int _number_printed; // 打印次数
};

int main()
{
    sys::CThreadPool<CTestThread> thread_pool; // 这里定义线程池实例

	try
	{
		// create可能抛出异常，所以需要捕获
		thread_pool.create(10); // 创建10个线程的线程池

		// 池线程创建成功后，并不会立即进行运行状态，而是处于等待状态，
		// 所以需要唤醒它们，方法如下：
		uint16_t thread_count = thread_pool.get_thread_count();
		CTestThread** test_thread_array = thread_pool.get_thread_array();
		for (uint16_t i=0; i<thread_count; ++i)
		{
			// 唤醒池线程，如果不调用wakeup，则CTestThread一直会等待唤醒它
			test_thread_array[i]->wakeup();
		}
		
		// 让CTestThread有足够的时间完成任务
		sys::CUtils::millisleep(5000);
		// 等待所有线程退出，然后销毁线程池
		thread_pool.destroy();
	}
	catch (sys::CSyscallException& ex)
	{
		// 将异常信息打印出来，方便定位原因
		printf("Create thread pool exception: %s.\n", ex.str().c_str());
	}
	
	return 0;
}
