#ifndef __CBENCHMARK_BENCHMARK_CHECK_H__
#define __CBENCHMARK_BENCHMARK_CHECK_H__

namespace BenchMark
{
#define BM_CHECK(b) \
    {               \
    }

// preserve whitespacing between operators for alignment
#define BM_CHECK_EQ(a, b) BM_CHECK((a) == (b))
#define BM_CHECK_NE(a, b) BM_CHECK((a) != (b))
#define BM_CHECK_GE(a, b) BM_CHECK((a) >= (b))
#define BM_CHECK_LE(a, b) BM_CHECK((a) <= (b))
#define BM_CHECK_GT(a, b) BM_CHECK((a) > (b))
#define BM_CHECK_LT(a, b) BM_CHECK((a) < (b))

#define BM_CHECK_FLOAT_EQ(a, b, eps) BM_CHECK(std::fabs((a) - (b)) < (eps))
#define BM_CHECK_FLOAT_NE(a, b, eps) BM_CHECK(std::fabs((a) - (b)) >= (eps))
#define BM_CHECK_FLOAT_GE(a, b, eps) BM_CHECK((a) - (b) > -(eps))
#define BM_CHECK_FLOAT_LE(a, b, eps) BM_CHECK((b) - (a) > -(eps))
#define BM_CHECK_FLOAT_GT(a, b, eps) BM_CHECK((a) - (b) > (eps))
#define BM_CHECK_FLOAT_LT(a, b, eps) BM_CHECK((b) - (a) > (eps))

} // namespace BenchMark

#endif // __CBENCHMARK_BENCHMARK_CHECK_H__