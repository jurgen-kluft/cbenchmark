#ifndef __CBENCHMARK_CHECKMACROS_H__
#define __CBENCHMARK_CHECKMACROS_H__

#include "cbenchmark/private/c_checks.h"
#include "cbenchmark/private/c_exception.h"
#include "cbenchmark/private/c_stringbuilder.h"


#ifdef CHECK
    #error BenchMark redefines CHECK
#endif

#define CHECK(value) \
    UT_TRY_BEGIN \
        if (!BenchMark::check(value!=0, true)) \
            testResults.onBenchMarkFailure(__FILE__, __LINE__, testName, "Expected 0, but actually "#value" == 0"); \
    UT_CATCH_ALL \
        testResults.onBenchMarkFailure(__FILE__, __LINE__, testName, "Unhandled exception in CHECK(" #value ")"); \
    UT_CATCH_END

#define CHECK_NULL(value) \
	UT_TRY_BEGIN \
		if (value != nullptr) \
			testResults.onBenchMarkFailure(__FILE__, __LINE__, testName, "Expected nullptr, but actually "#value" != nullptr"); \
    UT_CATCH_ALL \
		testResults.onBenchMarkFailure(__FILE__, __LINE__, testName, "Unhandled exception in CHECK(" #value ")"); \
	UT_CATCH_END

#define CHECK_NOT_NULL(value) \
	UT_TRY_BEGIN \
		if (value == nullptr) \
			testResults.onBenchMarkFailure(__FILE__, __LINE__, testName,"Expected NOT nullptr, but actually " #value" == nullptr"); \
    UT_CATCH_ALL \
		testResults.onBenchMarkFailure(__FILE__, __LINE__, testName, "Unhandled exception in CHECK(" #value ")"); \
	UT_CATCH_END

#define CHECK_TRUE(value) \
	UT_TRY_BEGIN \
		if (BenchMark::check((value)!=0, false)) \
			testResults.onBenchMarkFailure(__FILE__, __LINE__, testName, "Expected true but "#value" == false"); \
    UT_CATCH_ALL \
		testResults.onBenchMarkFailure(__FILE__, __LINE__, testName, "Unhandled exception in CHECK(" #value ")"); \
	UT_CATCH_END


#define CHECK_FALSE(value) \
	UT_TRY_BEGIN \
		if (BenchMark::check((value)!=0, true)) \
			testResults.onBenchMarkFailure(__FILE__, __LINE__, testName, "Expected false but "#value" == true"); \
    UT_CATCH_ALL \
		testResults.onBenchMarkFailure(__FILE__, __LINE__, testName, "Unhandled exception in CHECK(" #value ")"); \
	UT_CATCH_END

#define CHECK_EQUAL(expected, actual) \
    UT_TRY_BEGIN \
        BenchMark::checkEqual(testResults, expected, actual, testName, __FILE__, __LINE__, BenchMarkAllocator); \
    UT_CATCH_ALL \
        testResults.onBenchMarkFailure(__FILE__, __LINE__, testName, "Unhandled exception in CHECK_EQUAL(" #expected ", " #actual ")"); \
    UT_CATCH_END

#define CHECK_NOT_EQUAL(expected, actual) \
	UT_TRY_BEGIN \
		BenchMark::checkNotEqual(testResults, expected, actual, testName, __FILE__, __LINE__, BenchMarkAllocator); \
    UT_CATCH_ALL \
		testResults.onBenchMarkFailure(__FILE__, __LINE__, testName, "Unhandled exception in CHECK_EQUAL(" #expected ", " #actual ")"); \
	UT_CATCH_END

#define CHECK_EQUAL_T(expected, actual, msg) \
	UT_TRY_BEGIN \
		BenchMark::checkEqual(testResults, expected, actual, testName, __FILE__, __LINE__, BenchMarkAllocator); \
    UT_CATCH_ALL \
		testResults.onBenchMarkFailure(__FILE__, __LINE__, testName, msg); \
	UT_CATCH_END


#define CHECK_CLOSE(expected, actual, tolerance) \
    UT_TRY_BEGIN \
        BenchMark::checkClose(testResults, expected, actual, tolerance, testName, __FILE__, __LINE__, BenchMarkAllocator); \
    UT_CATCH_ALL \
        testResults.onBenchMarkFailure(__FILE__, __LINE__, testName, "Unhandled exception in CHECK_CLOSE(" #expected ", " #actual ")"); \
    UT_CATCH_END

#define CHECK_ARRAY_CLOSE(expected, actual, count, tolerance) \
    UT_TRY_BEGIN \
        BenchMark::checkArrayClose(testResults, expected, actual, count, tolerance, testName, __FILE__, __LINE__, BenchMarkAllocator); \
    UT_CATCH_ALL \
        testResults.onBenchMarkFailure(__FILE__, __LINE__, testName, "Unhandled exception in CHECK_ARRAY_CLOSE(" #expected ", " #actual ")"); \
    UT_CATCH_END

#define CHECK_THROW(expression, ExpectedExceptionType) \
    { \
        bool caught_ = false; \
        UT_TRY_BEGIN expression; \
        UT_CATCH(ExpectedExceptionType const&) caught_ = true; \
		UT_CATCH_END \
        if (!caught_) \
            testResults.onBenchMarkFailure(__FILE__, __LINE__, testName, "Expected exception: \"" #ExpectedExceptionType "\" not thrown"); \
    }

#define CHECK_ASSERT(expression) \
    if (!expression) { BenchMark::ReportAssert(__FILE__, __LINE__, #expression); }

#endif	///< __CBENCHMARK_CHECKMACROS_H__
