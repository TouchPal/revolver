#include "thread_class.h"

namespace cootek { namespace revolver{

thread_class::thread_class()
: m_status(STATUS_NOT_START)
, m_warning_queue_size(1000)
{
}

thread_class::~thread_class()
{
}

void thread_class::start(ex_callback ex)
{
    {
        boost::strict_lock<thread_class> guard(*this);
        if (m_status != STATUS_NOT_START)
            throw wrong_status();
        m_status = STATUS_STARTING;
        m_ex_cb = ex;
    }

    boost::promise<bool> init_promise;
    auto init_future = init_promise.get_future();
    m_thread = boost::make_shared<boost::thread>(boost::bind(
                &thread_class::thread_func,
                shared_from_this(),
                &init_promise));

    init_future.wait();

    if (init_future.has_exception())
    {
        m_thread->join();
        m_thread.reset();
        boost::rethrow_exception(init_future.get_exception_ptr());
    }
    else
    {
        BOOST_LOG_TRIVIAL(debug) << "start complete";
    }
}

void thread_class::thread_func(boost::promise<bool> *init_promise)
{
    BOOST_LOG_TRIVIAL(debug) << "thread start";
    {
        m_ev_loop = boost::make_shared<ev::dynamic_loop>();

        m_ev_tasks.set(*m_ev_loop);
        m_ev_tasks.set<thread_class, &thread_class::on_ev_task>(this);
        m_ev_tasks.start();

        try
        {
            thread_init();
        }
        catch (...)
        {

            BOOST_LOG_TRIVIAL(fatal) << "thread init failed";
            auto exptr = boost::current_exception();
            _thread_cleanup();
            {
                boost::strict_lock<thread_class> guard(*this);
                m_status = STATUS_STOPPED;
            }
            init_promise->set_exception(exptr);
            return;
        }
 
        {//update status
            boost::strict_lock<thread_class> guard(*this);
            m_status = STATUS_RUNNING;
        }
       
        BOOST_LOG_TRIVIAL(debug) << "notify start complete";
        init_promise->set_value(true);
    }

    if (m_ev_loop)
    {

        BOOST_LOG_TRIVIAL(debug) << "thread running";
        try
        {
            m_ev_loop->loop();
        }
        catch (...)
        {
            BOOST_LOG_TRIVIAL(fatal) << "exception in loop";
            if (m_ex_cb)
                m_ex_cb(std::current_exception());
        }
    }

    {//update status
        boost::strict_lock<thread_class> guard(*this);
        m_status = STATUS_STOPPING;
    }

    _thread_cleanup();

    {//update status
        boost::strict_lock<thread_class> guard(*this);
        m_status = STATUS_STOPPED;
    }
    BOOST_LOG_TRIVIAL(debug) << "thread terminated";
}

void thread_class::stop()
{
    {
        boost::strict_lock<thread_class> guard(*this);
        if (m_status == STATUS_STOPPED)
        {
            return;
        }
        else if (m_status == STATUS_RUNNING)
        {
            m_status = STATUS_STOPPING;
            assert(m_thread != NULL);
            thread_execute([this]()->void
                    {
                        m_ev_loop->break_loop(ev::ALL);
                    });
            m_tasks.close();
        }
        else if (m_status == STATUS_STOPPING)
        {
        }
        else
        {
            throw wrong_status();
        }

    }

    m_thread->join();

    BOOST_LOG_TRIVIAL(debug) << "stopped";
}

void thread_class::_thread_cleanup()
{
    BOOST_LOG_TRIVIAL(debug) << "thread cleanup";
    while (m_tasks.size())
    {
        work_type work;
        m_tasks.pull(work);
        work();
    }
 
    thread_cleanup();
    m_ev_tasks.stop();
}

void thread_class::thread_init()
{
}

void thread_class::thread_cleanup()
{
}

void thread_class::on_ev_task(ev::async &w, int revents)
{
    while (m_tasks.size())
    {
        work_type work;
        m_tasks.pull(work);
        work();
    }
}

}}//namespace
