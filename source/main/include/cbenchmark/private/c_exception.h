#ifndef __CBENCHMARK_EXCEPTION_H__
#define __CBENCHMARK_EXCEPTION_H__

#ifdef _NO_EXCEPTIONS

#define UT_TRY_BEGIN \
    {                \
        {
#define UT_CATCH(x) \
    }               \
    if (0)          \
    {
#define UT_CATCH_ALL \
    }                \
    if (0)           \
    {
#define UT_CATCH_END \
    }                \
    }

#define UT_THROW0()
#define UT_THROW1(x)
#define UT_THROW(x, y) x(y)

#else 

#define UT_TRY_BEGIN \
    try              \
    {
#define UT_CATCH(x) \
    }               \
    catch (x)       \
    {
#define UT_CATCH_ALL \
    }                \
    catch (...)      \
    {
#define UT_CATCH_END }

#define UT_THROW0() throw()
#define UT_THROW1(x) throw(x)
#define UT_THROW(x, y) throw x(y)

#endif 

#endif ///< __CBENCHMARK_EXCEPTIONH__
