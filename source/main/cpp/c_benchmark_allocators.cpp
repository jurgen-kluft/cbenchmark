#include "ccore/c_target.h"
#include "ccore/c_debug.h"

#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_benchmark_allocators.h"

#include <cstdlib>

namespace BenchMark
{
    void* MainAllocator::v_Allocate(unsigned int size, unsigned int alignment)
    {
        ASSERT(num_allocations_ >= 0);
        num_allocations_++;
#if defined(TARGET_MAC)
        return aligned_alloc(alignment, size);
#elif defined(TARGET_PC)
        return _aligned_malloc(alignment, size);
#endif
    }
    void MainAllocator::v_Deallocate(void* ptr)
    {
        ASSERT(num_allocations_ > 0);
        --num_allocations_;
#if defined(TARGET_MAC)
        free(ptr);
#elif defined(TARGET_PC)
        _aligned_free(ptr);
#endif
    }

    typedef unsigned char u8;

    ScratchAllocator::ScratchAllocator()
        : main_(nullptr)
        , buffer_begin_(nullptr)
        , buffer_end_(nullptr)
        , mark_(0)
    {
    }

    ScratchAllocator::~ScratchAllocator()
    {
        ASSERT(num_allocations_ == 0);
        if (main_ && buffer_begin_)
            main_->Deallocate(buffer_begin_);
    }

    void ScratchAllocator::Init(Allocator* alloc, u32 size)
    {
        main_            = alloc;
        buffer_begin_    = (u8*)alloc->Allocate(size);
        buffer_end_      = buffer_begin_ + size;
        buffer_[mark_]   = buffer_begin_;
        num_allocations_[mark_] = 0;
    }

    void* ScratchAllocator::v_Allocate(unsigned int size, unsigned int alignment)
    {
        u8* p = (u8*)((ncore::ptr_t)(buffer_ + (ncore::ptr_t)alignment - 1) & ~((ncore::ptr_t)alignment - 1));
        if ((p + size) > buffer_end_)
        {
            ASSERT(false);
            return nullptr;
        }
        num_allocations_[mark_]++;
        buffer_[mark_] = p + size;
        return p;
    }

    void ScratchAllocator::v_Deallocate(void* ptr)
    {
        ASSERT(ptr >= buffer_begin_ && ptr < buffer_end_);
        num_allocations_[mark_]--;
    }

    void ScratchAllocator::v_PushScope()
    {
        ++mark_;
        ASSERT(mark_ < (sizeof(buffer_) / sizeof(buffer_[0])));
        buffer_[mark_] = buffer_[mark_ - 1];
        num_allocations_[mark_] = 0;
    }

    void ScratchAllocator::v_PopScope()
    {
        ASSERT(num_allocations_[mark_] == 0);
        ASSERT(mark_ > 0);
        --mark_;
    }

    ForwardAllocator::ForwardAllocator()
        : main_(nullptr)
        , buffer_(nullptr)
        , buffer_begin_(nullptr)
        , buffer_end_(nullptr)
        , checkout_(0)
        , num_allocations_(0)
    {
    }

    ForwardAllocator::~ForwardAllocator()
    {
        ASSERT(num_allocations_ == 0);
        if (main_ && buffer_begin_)
            main_->Deallocate(buffer_begin_);
    }

    void ForwardAllocator::Init(Allocator* alloc, u32 size)
    {
        main_            = alloc;
        buffer_begin_    = (u8*)alloc->Allocate(size);
        buffer_end_      = buffer_begin_ + size;
        buffer_          = buffer_begin_;
        checkout_        = 0;
        num_allocations_ = 0;
    }

    void* ForwardAllocator::v_Checkout(unsigned int size, unsigned int alignment)
    {
        ASSERT(checkout_ >= 0);
        u8* p = (u8*)((ncore::ptr_t)(buffer_ + (ncore::ptr_t)alignment - 1) & ~((ncore::ptr_t)alignment - 1));
        if ((p + size) > buffer_end_)
            return nullptr;

        ASSERT(buffer_ < buffer_end_);
        num_allocations_++;
        checkout_++;
        return p;
    }

    void* ForwardAllocator::v_Allocate(unsigned int size, unsigned int alignment) 
    {
        void* ptr = v_Checkout(size, alignment);
        if (ptr == nullptr)
            return nullptr;    
        v_Commit((u8*)ptr + size);
        return ptr;
    }

    void ForwardAllocator::v_Commit(void* ptr)
    {
        buffer_ = (u8*)ptr;
        --checkout_;
        ASSERT(buffer_ < buffer_end_);
        ASSERT(checkout_ == 0);
    }

    void ForwardAllocator::v_Deallocate(void* ptr)
    {
        ASSERT(ptr >= buffer_begin_ && ptr < buffer_end_);
        --num_allocations_;
    }

} // namespace BenchMark
