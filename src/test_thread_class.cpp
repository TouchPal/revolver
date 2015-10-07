#include "inc.h"

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

#include "thread_class.h"
using cootek::revolver::thread_class;

class testc1 : public thread_class
{
public:
    testc1() :
        m_init(0),
        m_cleanup(0)
    {
    }

    void check1()
    {
        assert(m_init == 0);
        assert(m_cleanup == 0);
    }

    void check2()
    {
        assert(m_init == 1);
        assert(m_cleanup == 0);
    }

    void check3()
    {
        assert(m_init == 1);
        assert(m_cleanup == 1);
    }
protected:
    virtual void thread_init()
    {
        m_init++;
    }

    virtual void thread_cleanup()
    {
        m_cleanup++;
    }

private:
    int m_init;
    int m_cleanup;
};


BOOST_AUTO_TEST_CASE(thread_class_test1)
{
    auto t1 = boost::make_shared<testc1>();
    t1->check1();
    t1->start();
    t1->check2();
    t1->stop();
    t1->check3();
}


class testc2ex : public std::exception
{
};

class testc2 : public thread_class
{
public:
    testc2() :
        m_init(0),
        m_cleanup(0)
    {
    }

    void check1()
    {
        assert(m_init == 0);
        assert(m_cleanup == 0);
    }

    void check2()
    {
        assert(m_init == 0);
        assert(m_cleanup == 1);
    }

    void check3()
    {
        assert(m_init == 0);
        assert(m_cleanup == 1);
    }
protected:
    virtual void thread_init()
    {
        throw testc2ex();
    }

    virtual void thread_cleanup()
    {
        m_cleanup++;
    }

private:
    int m_init;
    int m_cleanup;
};

BOOST_AUTO_TEST_CASE(thread_class_test2)
{
    auto t2 = boost::make_shared<testc2>();
    t2->check1();
    bool ex = false;
    try
    {
        t2->start();
    }
    catch (std::exception &e)
    {
        ex = true;
    }
    BOOST_CHECK(ex);
    t2->check2();
    t2->stop();
    t2->check3();
}

class testc3 : public thread_class
{
public:
    testc3() :
        m_init(0),
        m_cleanup(0)
    {
    }

    void check1()
    {
        assert(m_init == 0);
        assert(m_cleanup == 0);
    }

    void check2()
    {
        assert(m_init == 1);
        assert(m_cleanup == 0);
    }

    void check3()
    {
        assert(m_init == 1);
        assert(m_cleanup == 1);
    }

    void do_ex()
    {
        m_async.send();
    }
protected:
    virtual void thread_init()
    {
        m_init++;
        m_async.set(ev_loop());
        m_async.set<testc3, &testc3::on_ev_exception>(this);
        m_async.start();
    }

    virtual void thread_cleanup()
    {
        m_cleanup++;
        m_async.stop();
    }

private:
    void on_ev_exception(ev::async &w, int revents)
    {
        std::cout << "throw exception" << std::endl;
        throw testc2ex();
    }
private:
    int m_init;
    int m_cleanup;

    ev::async m_async;
};

BOOST_AUTO_TEST_CASE(thread_class_test3)
{
    auto t2 = boost::make_shared<testc3>();
    bool has_ex = false;
    t2->check1();
    t2->start([&has_ex](std::exception_ptr p)->void {has_ex = true;});
    t2->check2();
    t2->do_ex();
    boost::this_thread::sleep_for(boost::chrono::milliseconds(50));
    assert(has_ex);
    t2->check3();
}

class testc4 : public thread_class
{
};

BOOST_AUTO_TEST_CASE(thread_class_test4)
{
    auto t2 = boost::make_shared<testc4>();
    t2->start();
    int c = 0;
    for (int i = 0; i < 100; ++i)
    {
        c += 1;
        t2->thread_execute([&c]() -> void {c += 1;});
    }

    t2->stop();
    BOOST_CHECK_EQUAL(c, 200);
}

BOOST_AUTO_TEST_CASE(thread_class_test5)
{
    auto t2 = boost::make_shared<testc4>();
    t2->start();
    int c = 0;
    for (int i = 0; i < 100; ++i)
    {
        c += 1;
        t2->thread_sync_execute<bool>([&c]() -> bool {c += 1; return true;});
    }

    BOOST_CHECK_EQUAL(c, 200);
}

BOOST_AUTO_TEST_CASE(thread_class_test6)
{
    auto t2 = boost::make_shared<testc4>();
    t2->start();
    int c = 0;
    for (int i = 0; i < 100; ++i)
    {
        c += 1;
        t2->thread_sync_execute([&c]() -> void {c += 1;});
    }

    BOOST_CHECK_EQUAL(c, 200);
}


