#ifndef __CBENCHMARK_ALLOCATOR_H__
#define __CBENCHMARK_ALLOCATOR_H__

#include "cbenchmark/private/c_types.h"

namespace BenchMark
{
    class Allocator
    {
    public:
        virtual ~Allocator() {}

        void*        Allocate(unsigned int size, unsigned int alignment = sizeof(void*)) { return v_Allocate(size, alignment); }
        unsigned int Deallocate(void* ptr) { return v_Deallocate(ptr); }

        template<typename T>
        T* Malloc(unsigned int size) { return (T*)v_Allocate(size, sizeof(void*)); }
        void Free(void* ptr) { v_Deallocate(ptr); }

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
        virtual void*        v_Allocate(unsigned int size, unsigned int alignment = sizeof(void*)) = 0;
        virtual unsigned int v_Deallocate(void* ptr)                                               = 0;
    };

    class NullAllocator : public Allocator
    {
    public:
        NullAllocator() {}

        virtual void*        v_Allocate(unsigned int size, unsigned int alignment) { return 0; }
        virtual unsigned int v_Deallocate(void* ptr) { return 0; }
    };

    class AllocatorTracker : public Allocator
    {
        Allocator* mAllocator;

    public:
        AllocatorTracker(Allocator* allocator)
            : mAllocator(allocator)
            , mNumAllocations(0)
        {
        }

        virtual void* v_Allocate(unsigned int size, unsigned int alignment)
        {
            IncNumAllocations();
            return mAllocator->Allocate(size, alignment);
        }

        virtual unsigned int v_Deallocate(void* ptr)
        {
            DecNumAllocations();
            return mAllocator->Deallocate(ptr);
        }

        void ResetNumAllocations() { mNumAllocations = 0; }
        void IncNumAllocations() { ++mNumAllocations; }
        void DecNumAllocations() { --mNumAllocations; }
        int  GetNumAllocations() const { return mNumAllocations; }
        int  mNumAllocations;
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
        ~Array() { if (m_alloc != nullptr) m_alloc->Deallocate(m_data); }

        void Init(Allocator* alloc, s32 size, s32 cap)
        {
            if (m_data != nullptr)
                m_alloc->Deallocate(m_data);

            m_alloc    = alloc;
            m_data     = (T*)alloc->Allocate(sizeof(T) * cap, sizeof(T));
            m_size     = size;
            m_capacity = cap;
        }

        void Copy(const Array<T>& other)
        {
            if (m_data != nullptr)
                m_alloc->Deallocate(m_data);

            m_alloc    = other.m_alloc;
            m_data     = (T*)m_alloc->Allocate(sizeof(T) * other.m_capacity, sizeof(T));
            m_size     = other.m_size;
            m_capacity = other.m_capacity;

            for (s32 i = 0; i < m_size; ++i)
                m_data[i] = other.m_data[i];
        }

        bool Empty() const { return m_size == 0; }

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
            if (m_size == m_capacity)
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

        void Erase(s32 i)
        {
            for (s32 j = i; j < m_size - 1; ++j)
                m_data[j] = m_data[j + 1];
            --m_size;
        }

        inline const T& Front() const { return m_data[i]; }
        inline T& Back() { return m_data[i]; }

        inline void Clear() { m_size = 0; }

        inline T&       operator[](s32 i) { return m_data[i]; }
        inline const T& operator[](s32 i) const { return m_data[i]; }

        s32 Size() const { return m_size; }
        s32 Capacity() const { return m_capacity; }

    private:
        Allocator* m_alloc;
        T*         m_data;
        s32        m_size;
        s32        m_capacity;
    };

} // namespace BenchMark

#endif // __CBENCHMARK_ALLOCATOR_H__
