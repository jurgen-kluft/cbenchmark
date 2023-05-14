#ifndef __CBENCHMARK_ALLOCATORS_H__
#define __CBENCHMARK_ALLOCATORS_H__

#include "cbenchmark/private/c_types.h"
#include "ccore/c_allocator.h"

namespace BenchMark
{
    class Allocator
    {
    public:
        virtual ~Allocator() {}

        void* Allocate(unsigned int size, unsigned int alignment = sizeof(void*)) { return v_Allocate(size, alignment); }
        void  Deallocate(void* ptr) { v_Deallocate(ptr); }

        template <typename T> T* Alloc(unsigned int size) { return (T*)v_Allocate(size, sizeof(void*)); }
        void                     Dealloc(void* ptr) { v_Deallocate(ptr); }

        // Construct and Destruct
        template <typename T, typename... Args> T* Construct(Args... args)
        {
            void* mem    = v_Allocate(sizeof(T), sizeof(void*));
            T*    object = new (mem) T(args...);
            return object;
        }

        template <typename T> void Destruct(T* object)
        {
            object->~T();
            v_Deallocate(object);
        }

    protected:
        virtual void* v_Allocate(unsigned int size, unsigned int alignment = sizeof(void*)) = 0;
        virtual void  v_Deallocate(void* ptr)                                               = 0;
    };

    class MainAllocator : public Allocator
    {
        s32 num_allocations_;

    public:
        MainAllocator()
            : num_allocations_(0)
        {
        }

    protected:
        virtual void* v_Allocate(unsigned int size, unsigned int alignment = sizeof(void*));
        virtual void  v_Deallocate(void* ptr);
    };

    class ForwardAllocator : public Allocator
    {
    public:
        ForwardAllocator();
        ~ForwardAllocator();

        void Initialize(Allocator* alloc, u32 size);
        void Reset();
        void Release();

        template <typename T> T* Checkout(unsigned int count, unsigned int alignment = sizeof(void*)) { return (T*)v_Checkout(count * sizeof(T), alignment); }
        void                     Commit(void* ptr) { v_Commit(ptr); }

    protected:
        virtual void* v_Checkout(unsigned int size, unsigned int alignment);
        virtual void  v_Commit(void* ptr);
        virtual void* v_Allocate(unsigned int size, unsigned int alignment);
        virtual void  v_Deallocate(void* ptr);

        Allocator* main_;
        u8*        buffer_;
        u8*        buffer_begin_;
        u8*        buffer_end_;
        s32        checkout_;
        s32        num_allocations_;
    };

    class ScratchAllocator : public Allocator
    {
        enum
        {
            MAX_MARK = 15
        };
        Allocator* main_;
        u8*        buffer_[MAX_MARK];
        u8*        buffer_begin_;
        u8*        buffer_end_;
        s32        allocs_[MAX_MARK];
        u32        checkout_[MAX_MARK];
        s32        mark_;

    public:
        ScratchAllocator();
        ~ScratchAllocator();

        void Initialize(Allocator* alloc, u32 size);
        void Reset();
        void Release();

        void PushScope() { v_PushScope(); }
        void PopScope() { v_PopScope(); }

        template <typename T> T* Checkout(unsigned int count, unsigned int alignment = sizeof(void*)) { return (T*)v_Checkout(count * sizeof(T), alignment); }
        void                     Commit(void* ptr) { v_Commit(ptr); }

    protected:
        virtual void* v_Allocate(unsigned int size, unsigned int alignment);
        virtual void  v_Deallocate(void* ptr);
        virtual void  v_PushScope();
        virtual void  v_PopScope();
        virtual void* v_Checkout(unsigned int size, unsigned int alignment);
        virtual void  v_Commit(void* ptr);
    };

    class ScopedScratchAllocator
    {
        ScratchAllocator* allocator_;

    public:
        ScopedScratchAllocator(ScratchAllocator* allocator)
            : allocator_(allocator)
        {
            allocator_->PushScope();
        }
        ~ScopedScratchAllocator() { allocator_->PopScope(); }
    };

#define USE_SCRATCH(allocator_name) ScopedScratchAllocator allocator_name##_scope(allocator_name)

    class NullAllocator : public Allocator
    {
    public:
        NullAllocator() {}

        virtual void* v_Allocate(unsigned int size, unsigned int alignment) { return 0; }
        virtual void  v_Deallocate(void* ptr) {}
    };

    class TrackingAllocator : public Allocator
    {
        Allocator* mAllocator;

    public:
        TrackingAllocator(Allocator* allocator)
            : mAllocator(allocator)
            , mNumAllocations(0)
        {
        }

        void ResetNumAllocations() { mNumAllocations = 0; }
        void IncNumAllocations() { ++mNumAllocations; }
        void DecNumAllocations() { --mNumAllocations; }
        int  GetNumAllocations() const { return mNumAllocations; }
        int  mNumAllocations;

    protected:
        virtual void* v_Allocate(unsigned int size, unsigned int alignment)
        {
            IncNumAllocations();
            return mAllocator->Allocate(size, alignment);
        }

        virtual void v_Deallocate(void* ptr)
        {
            DecNumAllocations();
            mAllocator->Deallocate(ptr);
        }
    };

} // namespace BenchMark

#endif // __CBENCHMARK_ALLOCATORS_H__