#if !_STLP_NO_IOSTREAMS
#include <deque>

#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

#include "reverse_lock.hpp"

#include "thread_pool.hpp"

namespace mstd {

class thread_pool::impl {
public:
    explicit impl(size_t t)
    {
        for(size_t i = 0; i != t; ++i)
            threads_.create_thread(boost::bind(&impl::execute, this));
    }

    void enqueue(const boost::function<void()> & f)
    {
        {
            boost::lock_guard<boost::mutex> lock(mutex_);
            queue_.push_back(f);
        }
        cond_.notify_one();
    }
private:
    void execute()
    {
        try {
            boost::unique_lock<boost::mutex> lock(mutex_);
            while(!boost::this_thread::interruption_requested())
            {
                cond_.wait(lock);
                if(!queue_.empty())
                {
                    boost::function<void()> f = queue_.front();
                    queue_.pop_front();
                    reverse_lock<boost::unique_lock<boost::mutex> > rlock(lock);
                    try {
                        f();
                    } catch(...) {
                    }
                }
            }
        } catch(boost::thread_interrupted&) {
        }
    }

    boost::mutex mutex_;
    boost::condition_variable cond_;
    boost::thread_group threads_;
    std::deque<boost::function<void()> > queue_;
};

thread_pool::thread_pool(size_t threads)
    : impl_(new impl(threads))
{
}

thread_pool::~thread_pool()
{
}

void thread_pool::enqueue(const boost::function<void()> & f)
{
    impl_->enqueue(f);
}

}
#endif
