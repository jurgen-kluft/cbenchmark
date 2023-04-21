#ifndef __CBENCHMARK_CONFIG_H__
#define __CBENCHMARK_CONFIG_H__

namespace BenchMark
{
    class Allocator;
    class BenchMarkObserver;

    class BenchMarkContext
    {
    public:
        BenchMarkContext();

        Allocator*         mAllocator;
        BenchMarkObserver* mObserver;
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
