#ifndef __CBENCHMARK_ARRAY_H__
#define __CBENCHMARK_ARRAY_H__

#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_allocators.h"

namespace BenchMark
{
    template <typename T> class Array
    {
    public:
        Array()
            : m_alloc(nullptr)
            , m_data(nullptr)
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

            m_alloc = alloc;
            m_data  = nullptr;
            if (cap > 0)
            {
                m_data = alloc->Alloc<T>(sizeof(T) * cap);
            }
            m_size     = size;
            m_capacity = cap;
        }

        void ClearReserve(s32 cap)
        {
            m_size = 0;
            if (m_capacity >= cap)
                return;

            m_capacity = 0;
            if (m_alloc != nullptr)
            {
                if (m_data != nullptr)
                    m_alloc->Deallocate(m_data);
                m_data     = m_alloc->Alloc<T>(sizeof(T) * cap);
                m_capacity = cap;
            }
        }

        void Clear() { m_size = 0; }

        void Release()
        {
            if (m_data != nullptr && m_alloc != nullptr)
                m_alloc->Deallocate(m_data);

            m_data     = nullptr;
            m_size     = 0;
            m_capacity = 0;
        }

        void Copy(const Array<T>& other)
        {
            if (other.Size() > m_capacity)
            {
                if (m_alloc == nullptr)
                    m_alloc = other.m_alloc;

                ClearReserve(other.Size());
            }

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

#endif // __CBENCHMARK_ARRAY_H__
