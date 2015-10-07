# revolver/thread_class
thread_class是一个很简单的工具类，但是在触宝的基础设施中得到了广泛的应用。他的功能是开一个工作线程，并且将工作放入工作线程来执行。
在thread_class中集成了libev的ev_loop，因此特别适用于一个工作线程、一个ev_loop的工作模式。

##thread_class主要有2种使用模式。
###1. 指令模式
主线程下发指令，由工作线程来执行。比如工作线程负责RTP的转发，而主线程下发从哪个端口，到哪个端口进行转发。
这种情形适合用thread_sync_execute来下发指令。 那么在thread_sync_execute返回时，指令已经成功下发了。
###2. 异步模式
主线程将一个具体的工作派发给工作线程来执行。工作线程在工作完成后，再通知主线程任务已经完成。
这种情形适合用thread_execute来派发工作。那么thread_execute会立即返回，不会阻塞主线程。

##启动工作线程
* 工作线程并不是在thread_class被构造时创建的，而是在thread_class::start被调用时被创建的。需要在工作线程中进行初始化的内容，可以放在重载的thread_init中，对应的清理函数放在重载的thread_cleanup中。
* start函数的参数是一个异常处理函数。注意，默认处理是忽略异常。
* start函数会在thread_init执行完成后再返回。因此，在thread_init中如果耗时很长的话，主线程也会被block很长时间。

##异常处理
* thread_init函数中抛出的异常，将会被转移到start函数中，在主线程抛出。
* thread_execute中，传入的task中抛出异常的话，start函数中传入的ex_callback将会被调用。
* thread_sync_execute中，传入的task中抛出异常的话，将会被转移到主线程抛出。
* thread_cleanup中不能抛出异常。

##终止工作线程
调用stop将终止工作线程。stop函数将会在工作线程完全终止后，再返回。因此请千万不要在工作线程中调用。


