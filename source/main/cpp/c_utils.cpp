#include "cbenchmark/private/c_utils.h"

#include <algorithm>
#include <cmath>

namespace BenchMark
{

    // kilo, Mega, Giga, Tera, Peta, Exa, Zetta, Yotta.
    const char kBigSIUnits[] = "kMGTPEZY";
    // Kibi, Mebi, Gibi, Tebi, Pebi, Exbi, Zebi, Yobi.
    const char kBigIECUnits[] = "KMGTPEZY";
    // milli, micro, nano, pico, femto, atto, zepto, yocto.
    const char kSmallSIUnits[] = "munpfazy";

    #define arraysize(array) (sizeof(array) / sizeof(array[0]))

    // We require that all three arrays have the same size.
    static_assert(arraysize(kBigSIUnits) == arraysize(kBigIECUnits), "SI and IEC unit arrays must be the same size");
    static_assert(arraysize(kSmallSIUnits) == arraysize(kBigSIUnits), "Small SI and Big SI unit arrays must be the same size");

    static const s64 kUnitsSize = arraysize(kBigSIUnits);

    static char* ToExponentAndMantissa(double val, double thresh, int precision, double one_k, char* mantissa, char const* mantissaEnd, s64* exponent)
    {
        if (val < 0)
        {
            mantissa = gStringAppend(mantissa, mantissaEnd, '-');
            val      = -val;
        }

        // Adjust threshold so that it never excludes things which can't be rendered in 'precision' digits.
        const double adjusted_threshold = std::max(thresh, 1.0 / std::pow(10.0, precision));
        const double big_threshold      = adjusted_threshold * one_k;
        const double small_threshold    = adjusted_threshold;

        // Values in ]simple_threshold,small_threshold[ will be printed as-is
        const double simple_threshold = 0.01;

        if (val > big_threshold)
        {
            // Positive powers
            double scaled = val;
            for (s32 i = 0; i < arraysize(kBigSIUnits); ++i)
            {
                scaled /= one_k;
                if (scaled <= big_threshold)
                {
                    // mantissa_stream << scaled;
                    *exponent = i + 1;
                    //*mantissa = mantissa_stream.str();
                    mantissa = gStringFormatAppend(mantissa, mantissaEnd, "%f", scaled);
                    return mantissa;
                }
            }
            // mantissa_stream << val;
            mantissa  = gStringFormatAppend(mantissa, mantissaEnd, "%f", val);
            *exponent = 0;
        }
        else if (val < small_threshold)
        {
            // Negative powers
            if (val < simple_threshold)
            {
                double scaled = val;
                for (s32 i = 0; i < arraysize(kSmallSIUnits); ++i)
                {
                    scaled *= one_k;
                    if (scaled >= small_threshold)
                    {
                        *exponent = -static_cast<s64>(i + 1);
                        mantissa  = gStringFormatAppend(mantissa, mantissaEnd, "%f", scaled);
                        return mantissa;
                    }
                }
            }
            mantissa  = gStringFormatAppend(mantissa, mantissaEnd, "%f", val);
            *exponent = 0;
        }
        else
        {
            mantissa  = gStringFormatAppend(mantissa, mantissaEnd, "%f", val);
            *exponent = 0;
        }
        return mantissa;
    }

    static char* ExponentToPrefix(s64 exponent, bool iec, char* prefix, const char* prefixEnd)
    {
        if (exponent == 0)
            return prefix;

        const s64 index = (exponent > 0 ? exponent - 1 : -exponent - 1);
        if (index >= kUnitsSize)
            return prefix;

        const char* array = (exponent > 0 ? (iec ? kBigIECUnits : kBigSIUnits) : kSmallSIUnits);
        if (iec)
        {
            prefix = gStringAppend(prefix, prefixEnd, array[index]);
            prefix = gStringAppend(prefix, prefixEnd, 'i');
            return prefix;
        }
        return gStringAppend(prefix, prefixEnd, array[index]);
    }

    static char* ToBinaryStringFullySpecified(char* str, const char* strEnd, double value, double threshold, int precision, double one_k = 1024.0)
    {
        s64 exponent;
        str = ToExponentAndMantissa(value, threshold, precision, one_k, str, strEnd, &exponent);
        return ExponentToPrefix(exponent, false, str, strEnd);
    }

    char* gHumanReadableNumber(char* dst, const char* dstEnd, double n, double one_k) 
    {
        return ToBinaryStringFullySpecified(dst, dstEnd, n, 1.1, 1, one_k);
    }

    void gSetWidthFormat(char* format, int width)
    {
        int i       = 0;
        format[i++] = '%';
        if (width > 99)
            format[i++] = '0' + (width / 100);
        if (width > 9)
            format[i++] = '0' + ((width % 100) / 10);
        if (width > 0)
            format[i++] = '0' + (width % 10);
        format[i++] = 's';
        format[i++] = '\0';
    }

    char* gFormatTime(double time, char* str, const char* str_end)
    {
        // For the time columns of the console printer 13 digits are reserved. One of
        // them is a space and max two of them are the time unit (e.g ns). That puts
        // us at 10 digits usable for the number.
        // Align decimal places...
        if (time < 1.0)
            return gStringFormatAppend(str, str_end, "%10.3f", time);
        if (time < 10.0)
            return gStringFormatAppend(str, str_end, "%10.2f", time);
        if (time < 100.0)
            return gStringFormatAppend(str, str_end, "%10.1f", time);

        // Assuming the time is at max 9.9999e+99 and we have 10 digits for the
        // number, we get 10-1(.)-1(e)-1(sign)-2(exponent) = 5 digits to print.
        if (time > 9999999999 /*max 10 digit number*/)
            return gStringFormatAppend(str, str_end, "%1.4e", time);
        return gStringFormatAppend(str, str_end, "%10.0f", time);
    }

    bool gIsZero(double x)
    {
        return x > -1e-9 && x < 1e-9;
    }

    char* gStringAppend(char* dst, const char* dstEnd, const char* src)
    {
        while (dst < dstEnd && *src != '\0')
        {
            *dst++ = *src++;
        }
        return dst;
    }

    char* gStringAppend(char* dst, const char* dstEnd, char src)
    {
        if (dst < dstEnd)
            *dst++ = src;
        return dst;
    }

    void gStringCopy(char* dst, const char* src, int max)
    {
        int i = 0;
        while (i < max && src[i] != 0)
        {
            dst[i] = src[i];
            ++i;
        }

        if (i < max)
            dst[i] = '\0';
    }

    const char* gStringFind(const char* src, const char* findstr)
    {
        if (*findstr == '\0')
            return src;

        for (; *src != '\0'; ++src)
        {
            if (*src == *findstr)
            {
                /// Matched starting char -- loop through remaining chars.
                const char *h, *n;
                for (h = src, n = findstr; *h != '\0' && *n != '\0'; ++h, ++n)
                {
                    if (*h != *n)
                        break;
                }

                if (*n == '\0') ///< matched all of 'needle' to null termination
                    return src;
            }
        }
        return 0;
    }

    char* gStringFind(char* src, const char* findstr)
    {
        if (*findstr == '\0')
            return src;

        for (; *src != '\0'; ++src)
        {
            if (*src == *findstr)
            {
                /// Matched starting char -- loop through remaining chars.
                const char *h, *n;
                for (h = src, n = findstr; *h != '\0' && *n != '\0'; ++h, ++n)
                {
                    if (*h != *n)
                        break;
                }

                if (*n == '\0') ///< matched all of 'needle' to null termination
                    return src;
            }
        }
        return 0;
    }

    int gStringLength(const char* str)
    {
        int i = 0;
        while (str[i] != 0)
            i++;
        return i;
    }

    s32 gCompareStrings(char const* expected, char const* actual)
    {
        if (expected == actual)
            return 0;
        if (expected == nullptr && actual != nullptr)
            return -1;
        if (expected != nullptr && actual == nullptr)
            return 1;
        do
        {
            const char e = *expected;
            const char a = *actual;
            if (e < a)
                return -1;
            else if (e > a)
                return 1;
            else if (e == '\0')
                break; // this also means that a == '\0'
            expected++;
            actual++;
        } while (true);
        return 0;
    }

    bool gAreStringsEqual(char const* expected, char const* actual) { return gCompareStrings(expected, actual) == 0; }

} // namespace BenchMark
