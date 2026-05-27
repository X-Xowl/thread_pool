# thread_pool
A thread pool based on C++23 一个基于C++23标准实现的线程池  

To use it, you only need to include the header file. The compiler needs to be set with the following options: "-std=c++23" and "-lstdc++exp"  
Currently, it has only been tested on the gcc compiler. Please use the latest version of the compiler  
仅需要包含头文件即可使用，需要编译器开启如下选项 "-std=c++23" "-lstdc++exp"  
目前仅在gcc编译器上测试过，请使用最新版本的编译器  
使用方法：thread_pool pool(int thread_count)，创建线程池并传入需要创建的线程数  
然后     pool.submit()  传入你要执行的函数即可  
使用     pool.wait() 可以阻塞等待任务执行完毕
