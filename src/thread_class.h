#ifndef THREAD_CLASS_H
#define THREAD_CLASS_H 
#include "inc.h"

namespace cootek { namespace revolver{

class thread_class :
    public boost::lockable_adapter<boost::recursive_mutex>,
    public boost::enable_shared_from_this<thread_class>
{
    typedef typename boost::executors::work work_type;
    typedef boost::sync_queue<work_type> works_type;
public:
    typedef boost::function<void(std::exception_ptr)> ex_callback;


    static const int STATUS_NOT_START = 0;
    static const int STATUS_STARTING = 1;
    static const int STATUS_RUNNING = 2;
    static const int STATUS_STOPPING = 3;
    static const int STATUS_STOPPED = 4;

public:
    thread_class();
    virtual ~thread_class();

    thread_class(const thread_class &) = delete;
    thread_class &operator = (const thread_class &) = delete;

    /* Start the working thread.
     * ex is the callback function which will be called when exception occur.
     * default ex function just ignore the exception. 
     * This function won't return until thread_init return.
     * All exceptions throwed from thread_init will rethrow in start function.
     * */
    void start(ex_callback ex = ex_callback());

    /* Request the working thread stop.
     * If call stop before normal running, throw wrong_status exception.
     * This function will blocked until the working thread terminated.
     */
    void stop();

    inline void set_warning_queue_size(unsigned int s) {m_warning_queue_size = s;}

    /* Execute task in working thread. 
     * This function will return immediately after place task in queue.
     */
    template<typename T>
    inline void thread_execute(T task);

    /*Execute task in working thread.
     * Caller Thread will block until task execute complete. 
     * So this function should never be called in working thread.
     * R is the return value type of function task.
     * You can throw exeception in function task. It will be catched in working thread and be rethrowed in thread_sync_execute.
     */
    template<typename R, typename T>
    inline R thread_sync_execute(T task);

    /* Special version for void return task.
     */
    template<typename T>
    inline void thread_sync_execute(T task);



protected:
    /*This function will be called in working thread before status change to RUNNING.
     * This function will be called after initialize ev_loop.
     * So It can be used to initialize thread local storage or ev objects.
     */
    virtual void thread_init();

    /* This function will be called in working thread after ev_loop stoped.
     * It can be used for clear thread local storage or ev objects.
     */
    virtual void thread_cleanup();

    /* Get ev loop*/
    inline ev::loop_ref& ev_loop() {return *m_ev_loop;}

    inline bool is_working()
    {
        boost::strict_lock<thread_class> guard(*this);
        return m_status == STATUS_RUNNING;
    }

    inline bool is_stopping() 
    {
        boost::strict_lock<thread_class> guard(*this);
        return m_status == STATUS_STOPPING;
    }

private:
    virtual void thread_func(boost::promise<bool> *init_promise);
    void on_ev_task(ev::async &w, int revents);

    void _thread_cleanup();


private:
    int                                 m_status;
    unsigned int                        m_warning_queue_size;
    boost::shared_ptr<boost::thread>    m_thread;
    boost::shared_ptr<ev::dynamic_loop> m_ev_loop;

    ev::async                           m_ev_tasks;
    works_type                          m_tasks;

    ex_callback                         m_ex_cb;
};

template<typename T>
void thread_class::thread_execute(T task)
{
    if (m_tasks.size() > m_warning_queue_size)
        BOOST_LOG_TRIVIAL(error) << "too many tasks in work thread queue.";
    
    m_tasks.push(work_type(task));
    m_ev_tasks.send();
}

template<typename R, typename T>
R thread_class::thread_sync_execute(T task)
{
    boost::promise<R> r_promise;
    auto r_future = r_promise.get_future();
    thread_execute([&r_promise, &task]()->void
            {
                try
                {
                    r_promise.set_value(task());
                }
                catch (...)
                {
                    r_promise.set_exception(boost::current_exception());
                }
            });


    r_future.wait();
    if (r_future.has_exception())
    {
        boost::rethrow_exception(r_future.get_exception_ptr());
    }
    return r_future.get();
}

template<typename T>
void thread_class::thread_sync_execute(T task)
{
    boost::promise<bool> r_promise;
    auto r_future = r_promise.get_future();
    thread_execute([&r_promise, &task]()->void
            {
                try
                {
                    task();
                    r_promise.set_value(true);
                }
                catch (...)
                {
                    r_promise.set_exception(boost::current_exception());
                }
            });


    r_future.wait();
    if (r_future.has_exception())
    {
        boost::rethrow_exception(r_future.get_exception_ptr());
    }
}


class wrong_status : public std::exception
{
};

}} //namespace

#endif /* THREAD_CLASS_H */
