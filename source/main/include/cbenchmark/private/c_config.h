#ifndef __CBENCHMARK_CONFIG_H__
#define __CBENCHMARK_CONFIG_H__

namespace BenchMark
{
    class BenchMarkAllocator
    {
    public:
        virtual ~BenchMarkAllocator() {}

        virtual void*        Allocate(unsigned int size, unsigned int alignment = sizeof(void*)) = 0;
        virtual unsigned int Deallocate(void* ptr)                                               = 0;
    };

    class BenchMarkAllocatorEx : public BenchMarkAllocator
    {
        BenchMarkAllocator* mAllocator;

    public:
        BenchMarkAllocatorEx(BenchMarkAllocator* allocator)
            : mAllocator(allocator)
            , mNumAllocations(0)
        {
        }

        virtual void* Allocate(unsigned int size, unsigned int alignment)
        {
            IncNumAllocations();
            return mAllocator->Allocate(size, alignment);
        }

        virtual unsigned int Deallocate(void* ptr)
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

    class BenchMarkObserver
    {
    public:
        virtual ~BenchMarkObserver() {}

        virtual void BeginSuite(const char* filename, const char* suite_name) = 0;
        virtual void EndSuite()                                               = 0;

        virtual void BeginFixture(const char* filename, const char* suite_name, const char* fixture_name) = 0;
        virtual void EndFixture()                                                                         = 0;

        virtual void BeginBenchMark(const char* filename, const char* suite_name, const char* fixture_name, const char* test_name) = 0;
        virtual void EndBenchMark()                                                                                                = 0;
    };

    class BenchMarkContext
    {
    public:
        BenchMarkContext();

        BenchMarkAllocator* mAllocator;
        BenchMarkObserver*  mObserver;
    };

    class NullAllocator : public BenchMarkAllocator
    {
    public:
        NullAllocator() {}

        virtual void*        Allocate(unsigned int size, unsigned int alignment) { return 0; }
        virtual unsigned int Deallocate(void* ptr) { return 0; }
        void                 Release() {}
    };

    class NullObserver : public BenchMarkObserver
    {
    public:
        virtual void BeginSuite(const char* filename, const char* suite_name) {}
        virtual void EndSuite() {}

        virtual void BeginFixture(const char* filename, const char* suite_name, const char* fixture_name) {}
        virtual void EndFixture() {}

        virtual void BeginBenchMark(const char* filename, const char* suite_name, const char* fixture_name, const char* test_name) {}
        virtual void EndBenchMark() {}
    };
} // namespace BenchMark

#endif ///< __CBENCHMARK_CONFIG_H__
