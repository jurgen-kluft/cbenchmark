#include "ccore/c_target.h"
#include "ccore/c_allocator.h"

#include "cunittest/cunittest.h"

UNITTEST_SUITE_LIST

namespace ncore
{
    // Our own assert handler
    class UnitTestAssertHandler : public ncore::asserthandler_t
    {
    public:
        UnitTestAssertHandler() { NumberOfAsserts = 0; }

        virtual bool handle_assert(u32& flags, const char* fileName, s32 lineNumber, const char* exprString, const char* messageString)
        {
            UnitTest::ReportAssert(exprString, fileName, lineNumber);
            NumberOfAsserts++;
            return false;
        }

        ncore::s32 NumberOfAsserts;
    };

    class TestAllocator : public alloc_t
    {
        UnitTest::TestAllocator* mAllocator;

    public:
        TestAllocator(UnitTest::TestAllocator* allocator)
            : mAllocator(allocator)
        {
        }

        virtual void* v_allocate(u32 size, u32 alignment) { return mAllocator->Allocate(size, alignment); }
        virtual u32   v_deallocate(void* mem) { return mAllocator->Deallocate(mem); }
        virtual void  v_release() {}
    };
} // namespace ncore

bool gRunUnitTest(UnitTest::TestReporter& reporter, UnitTest::TestContext& context)
{
#ifdef TARGET_DEBUG
    ncore::UnitTestAssertHandler assertHandler;
    ncore::gSetAssertHandler(&assertHandler);
#endif

    ncore::TestAllocator testAllocator(context.mAllocator);

    int r = UNITTEST_SUITE_RUN(context, reporter, cUnitTest);

    return r == 0;
}
