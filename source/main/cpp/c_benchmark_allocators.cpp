#include "ccore/c_target.h"
#include "ccore/c_debug.h"
#include "ccore/c_allocator.h"

#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_benchmark_allocators.h"

#include <cstdlib>

namespace BenchMark
{
    void* MainAllocator::v_Allocate(s64 size, unsigned int alignment)
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
        ASSERT(mark_ == 0);
        ASSERT(checkout_[mark_] == 0);
        ASSERT(allocs_[mark_] == 0);
        if (main_ && buffer_begin_)
            main_->Deallocate(buffer_begin_);
    }

    void ScratchAllocator::Initialize(Allocator* alloc, s64 size)
    {
        main_         = alloc;
        buffer_begin_ = alloc->Alloc<u8>(size);
        buffer_end_   = buffer_begin_ + size;
        Reset();
    }

    void ScratchAllocator::Reset()
    {
        mark_ = 0;
        for (int i = 0; i < MAX_MARK; ++i)
        {
            buffer_[i]   = nullptr;
            allocs_[i]   = 0;
            checkout_[i] = 0;
        }
        buffer_[mark_]   = buffer_begin_;
        allocs_[mark_]   = 0;
        checkout_[mark_] = 0;
    }

    void ScratchAllocator::Release()
    {
        main_->Deallocate(buffer_begin_);
        buffer_begin_ = nullptr;
        buffer_end_   = nullptr;
        Reset();
    }

    void ScratchAllocator::v_PushScope()
    {
        // Going into a new scope we should consider an active checkout
        ++mark_;
        ASSERT(mark_ < (sizeof(buffer_) / sizeof(buffer_[0])));
        buffer_[mark_]   = buffer_[mark_ - 1] + checkout_[mark_ - 1];
        allocs_[mark_]   = 0;
        checkout_[mark_] = 0;
    }

    void ScratchAllocator::v_PopScope()
    {
        ASSERT(checkout_[mark_] == 0);
        ASSERT(allocs_[mark_] == 0);
        ASSERT(mark_ > 0);
        --mark_;
    }

    void* ScratchAllocator::v_Checkout(s64 size, unsigned int alignment)
    {
        // Doing a checkout requires that any other checkouts are committed
        ASSERT(checkout_[mark_] == 0);
        u8* p = ncore::g_align_ptr<u8>(buffer_[mark_], alignment);
        if ((p + size) > buffer_end_)
            return nullptr;

        ASSERT(buffer_[mark_] < buffer_end_);
        allocs_[mark_]++;
        checkout_[mark_] = size;
        return p;
    }

    void* ScratchAllocator::v_Allocate(s64 size, unsigned int alignment)
    {
        void* ptr = v_Checkout(size, alignment);
        if (ptr == nullptr)
            return nullptr;
        v_Commit((u8*)ptr + size);
        return ptr;
    }

    void ScratchAllocator::v_Deallocate(void* ptr)
    {
        ASSERT(ptr >= buffer_begin_ && ptr < buffer_end_);
        allocs_[mark_]--;
    }

    void ScratchAllocator::v_Commit(void* ptr)
    {
        checkout_[mark_] = 0;
        buffer_[mark_]   = (u8*)ptr;
        ASSERT(buffer_[mark_] < buffer_end_);
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

    void ForwardAllocator::Initialize(Allocator* alloc, s64 size)
    {
        main_            = alloc;
        buffer_begin_    = alloc->Alloc<u8>(size);
        buffer_end_      = buffer_begin_ + size;
        buffer_          = buffer_begin_;
        checkout_        = 0;
        num_allocations_ = 0;
    }

    void ForwardAllocator::Reset()
    {
        ASSERT(buffer_ == buffer_begin_);
        buffer_          = buffer_begin_;
        checkout_        = 0;
        num_allocations_ = 0;
    }

    void ForwardAllocator::Release()
    {
        main_->Deallocate(buffer_begin_);
        buffer_begin_    = nullptr;
        buffer_end_      = nullptr;
        buffer_          = nullptr;
        checkout_        = 0;
        num_allocations_ = 0;
    }

    struct ForwardAllocationHeader
    {
        u64 magic;
        void* ptr;
        s64 size;
    };

    void* ForwardAllocator::v_Checkout(s64 size, unsigned int alignment)
    {
        ASSERT(checkout_ >= 0);
        u8* p = ncore::g_align_ptr(buffer_, alignment);
        if ((p + size) > buffer_end_)
            return nullptr;

        header_       = (ForwardAllocationHeader*)p;
        p             = (u8*)(header_ + 1);
        header_->magic = 0XDEADBEEFDEADBEEFULL;
        header_->ptr  = (void*)(p);
        header_->size = size;

        ASSERT(buffer_ < buffer_end_);
        num_allocations_++;
        checkout_++;
        return p;
    }

    void* ForwardAllocator::v_Allocate(s64 size, unsigned int alignment)
    {
        void* ptr = v_Checkout(size, alignment);
        if (ptr == nullptr)
            return nullptr;
        v_Commit((u8*)ptr + size);
        return ptr;
    }

    void ForwardAllocator::v_Commit(void* ptr)
    {
        header_->size = (u8*)ptr - (u8*)(header_->ptr);
        buffer_       = (u8*)ptr;
        --checkout_;
        ASSERT(buffer_ < buffer_end_);
        ASSERT(checkout_ == 0);
    }

    void ForwardAllocator::v_Deallocate(void* ptr)
    {
        // Check the header for double free's and validate the pointer
        ForwardAllocationHeader* header = (ForwardAllocationHeader*)ptr - 1;
        if (header->magic == 0XDEADBEEFDEADBEEFULL)
        {
            if (header->ptr != ptr)
            {
                return;
            }
            if (header->size <= 0)
            {
                return;
            }
        }
        else 
        {
            // corrupted ?
            return;
        }

        ASSERT(header->magic == 0XDEADBEEFDEADBEEFULL);
        ASSERT(header->ptr == ptr);
        ASSERT(header->size > 0);

        // Empty this header by just setting size to 0
        header->size  = 0;

        ASSERT(ptr >= buffer_begin_ && ptr < buffer_end_);
        if (num_allocations_ == 0)
        {
            return;
        }
        ASSERT(num_allocations_ > 0);
        --num_allocations_;
    }

} // namespace BenchMark
