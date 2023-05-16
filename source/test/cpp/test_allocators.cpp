#include "ccore/c_target.h"
#include "cbenchmark/cbenchmark.h"
#include "cbenchmark/private/c_benchmark_results.h"
#include "cbenchmark/private/c_benchmark_state.h"
#include "cbenchmark/private/c_benchmark_unit.h"
#include "cbenchmark/private/c_benchmark_reporter.h"
#include "cbenchmark/private/c_benchmark_allocators.h"

#include "cunittest/cunittest.h"

#include <string>

using namespace ncore;

namespace BenchMark
{
    class TestAllocator : public Allocator
    {
    public:
        s32                      num_allocations;
        UnitTest::TestAllocator* allocator;

        TestAllocator(UnitTest::TestAllocator* a)
            : num_allocations(0)
            , allocator(a)
        {
        }

        DCORE_CLASS_PLACEMENT_NEW_DELETE

        virtual void* v_Allocate(s64 size, unsigned int alignment);
        virtual void  v_Deallocate(void* ptr);
    };

    void* TestAllocator::v_Allocate(s64 size, unsigned int alignment) { return nullptr; }

    void TestAllocator::v_Deallocate(void* ptr) {}

} // namespace BenchMark

UNITTEST_SUITE_BEGIN(test_allocators)
{
    UNITTEST_FIXTURE(main)
    {
        UNITTEST_FIXTURE_SETUP() {}
        UNITTEST_FIXTURE_TEARDOWN() {}

        struct TestStruct
        {
            TestStruct(float a, float b, int c)
                : a(a)
                , b(b)
                , c(c)
            {
            }
            float a;
            float b;
            int   c;
        };

        // UNITTEST_TEST(test)
        // {
        //     BenchMark::TestAllocator ta(TestAllocator);

        //     BenchMark::ForwardAllocator a;
        //     a.Initialize(&ta, 1024);

        //     // so we want to test all the functions of the forward allocator
        //     TestStruct* t = a.Construct<TestStruct>(1.0f, 2.0f, 3);
        // }
    }
}
UNITTEST_SUITE_END
