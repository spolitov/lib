#pragma once

namespace mstd {

class linked_allocator {
public:
    explicit linked_allocator(size_t size)
        : size_(size), head_(0) {}

    ~linked_allocator()
    {
        void * cur = head_;
        while(cur) {
            void * n = next(cur);
            :: operator delete(cur);
            cur = n;
        }
    }

    void * allocate()
    {
        if(head_)
        {
            void * result = head_;
            head_ = next(result);
            return result;
        } else {
            return ::operator new(size_);
        }
    }
    
    void deallocate(void * ptr)
    {
        next(ptr) = head_;
        head_ = ptr;
    }
private:
    static void *& next(void * addr)
    {
        return *static_cast<void**>(addr);
    }

    size_t size_;
    void * head_;
};

}
