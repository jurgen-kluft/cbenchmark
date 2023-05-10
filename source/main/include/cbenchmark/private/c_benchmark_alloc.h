#ifndef __CBENCHMARK_ALLOCATOR_H__
#define __CBENCHMARK_ALLOCATOR_H__

#include "cbenchmark/private/c_types.h"

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

    class ForwardAllocator
    {
    public:
        template <typename T> T* Checkout(unsigned int size, unsigned int alignment = sizeof(void*)) { return v_Checkout(size, alignment); }
        void                     Commit(void* ptr) { v_Commit(ptr); }

        template <typename T> T* Alloc(unsigned int size, unsigned int alignment = sizeof(void*))
        {
            void* ptr = v_Checkout(size, alignment);
            v_Commit(ptr);
            return ptr;
        }
        void Dealloc(void* ptr) { v_Deallocate(ptr); }

        // Construct and Destruct
        template <typename T, typename... Args> T* Construct(Args... args)
        {
            void* mem = v_Checkout(sizeof(T), sizeof(void*));
            v_Commit(mem);
            T* object = new (mem) T(args...);
            return object;
        }

        template <typename T> void Destruct(T* object) { object->~T(); }

    protected:
        virtual void* v_Checkout(unsigned int size, unsigned int alignment) = 0;
        virtual void  v_Commit(void* ptr)                                   = 0;
        virtual void  v_Deallocate(void* ptr)                               = 0;
    };

    class ScratchAllocator : public Allocator
    {
    public:
        inline void Reset() { v_Reset(); }

    protected:
        virtual void v_Reset() = 0;
    };

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

    template <typename T> class Array
    {
    public:
        Array()
            : m_data(nullptr)
            , m_size(0)
            , m_capacity(0)
        {
        }
        ~Array()
        {
            if (m_alloc != nullptr)
                m_alloc->Deallocate(m_data);
        }

        void Init(Allocator* alloc, s32 size, s32 cap)
        {
            if (m_data != nullptr)
                m_alloc->Deallocate(m_data);

            m_alloc    = alloc;
            m_data     = alloc->Alloc<T>(sizeof(T) * cap);
            m_size     = size;
            m_capacity = cap;
        }

        void ClearReserve(s32 cap)
        {
            if (m_capacity >= cap)
            {
                m_size = 0;
                return;
            }

            Release();

            m_data     = m_alloc->Alloc<T>(sizeof(T) * cap);
            m_capacity = cap;
        }

        void Clear() { m_size = 0; }

        void Release()
        {
            if (m_data != nullptr)
                m_alloc->Deallocate(m_data);

            m_data     = nullptr;
            m_size     = 0;
            m_capacity = 0;
        }

        void Copy(const Array<T>& other)
        {
            if (other.Size() > m_capacity)
                ClearReserve(other.Size());

            for (s32 i = 0; i < other.Size(); ++i)
                m_data[i] = other.m_data[i];
            m_size = other.Size();
        }

        bool Empty() const { return m_size == 0; }
        bool Full() const { return m_size == m_capacity; }

        T*       Begin() { return m_data; }
        T const* Begin() const { return m_data; }
        T*       End() { return m_data + m_size; }
        T const* End() const { return m_data + m_size; }

        T& Alloc()
        {
            m_size++;
            return m_data[m_size - 1];
        }

        bool PushBack(Array<T>& other)
        {
            if (m_size + other.m_size > m_capacity)
                return false;
            for (s32 i = 0; i < other.m_size; ++i)
                m_data[m_size++] = other.m_data[i];
            return true;
        }

        bool PushBack(const T& value)
        {
            if (Full())
                return false;
            m_data[m_size++] = value;
            return true;
        }

        bool PopBack()
        {
            if (m_size > 0)
            {
                --m_size;
                return true;
            }
            return false;
        }

        s32 Find(const T& value) const
        {
            for (s32 i = 0; i < m_size; ++i)
            {
                if (m_data[i] == value)
                    return i;
            }
            return -1;
        }

        s32             Size() const { return m_size; }
        s32             Capacity() const { return m_capacity; }
        inline const T& Front() const { return m_data[0]; }
        inline T&       Back() { return m_data[m_size - 1]; }
        inline T&       At(s32 i) { return m_data[i]; }
        inline const T& At(s32 i) const { return m_data[i]; }
        inline T&       operator[](s32 i) { return m_data[i]; }
        inline const T& operator[](s32 i) const { return m_data[i]; }

    private:
        Allocator* m_alloc;
        T*         m_data;
        s32        m_size;
        s32        m_capacity;
    };

} // namespace BenchMark

#endif // __CBENCHMARK_ALLOCATOR_H__
