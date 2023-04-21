#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_stringbuilder.h"
#include "cbenchmark/private/c_utils.h"
#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_alloc.h"

namespace BenchMark
{
    TextStreamWriter& TextStreamWriter::operator<<(const char* txt)
    {
        while (*txt && !mStream.isEof())
        {
            *mStream.stream++ = *txt++;
        }
        return *this;
    }
    TextStreamWriter& TextStreamWriter::operator<<(const void* p)
    {
        mStream.stream = gStringFormatAppend(mStream.stream, mStream.eos, "%0X", (u64)p);
        return *this;
    }
    TextStreamWriter& TextStreamWriter::operator<<(char const n)
    {
        mStream.stream = gStringFormatAppend(mStream.stream, mStream.eos, "%i", n);
        return *this;
    }
    TextStreamWriter& TextStreamWriter::operator<<(short const n)
    {
        mStream.stream = gStringFormatAppend(mStream.stream, mStream.eos, "%i", n);
        return *this;
    }
    TextStreamWriter& TextStreamWriter::operator<<(int const n)
    {
        mStream.stream = gStringFormatAppend(mStream.stream, mStream.eos, "%i", n);
        return *this;
    }
    TextStreamWriter& TextStreamWriter::operator<<(long long const n)
    {
        mStream.stream = gStringFormatAppend(mStream.stream, mStream.eos, "%i", n);
        return *this;
    }
    TextStreamWriter& TextStreamWriter::operator<<(long const n)
    {
        mStream.stream = gStringFormatAppend(mStream.stream, mStream.eos, "%i", n);
        return *this;
    }
    TextStreamWriter& TextStreamWriter::operator<<(unsigned char const n)
    {
        mStream.stream = gStringFormatAppend(mStream.stream, mStream.eos, "%i", n);
        return *this;
    }
    TextStreamWriter& TextStreamWriter::operator<<(unsigned short const n)
    {
        mStream.stream = gStringFormatAppend(mStream.stream, mStream.eos, "%i", n);
        return *this;
    }
    TextStreamWriter& TextStreamWriter::operator<<(unsigned int const n)
    {
        mStream.stream = gStringFormatAppend(mStream.stream, mStream.eos, "%i", n);
        return *this;
    }
    TextStreamWriter& TextStreamWriter::operator<<(unsigned long long const n)
    {
        mStream.stream = gStringFormatAppend(mStream.stream, mStream.eos, "%i", n);
        return *this;
    }
    TextStreamWriter& TextStreamWriter::operator<<(unsigned long const n)
    {
        mStream.stream = gStringFormatAppend(mStream.stream, mStream.eos, "%i", n);
        return *this;
    }
    TextStreamWriter& TextStreamWriter::operator<<(float const f)
    {
        mStream.stream = gStringFormatAppend(mStream.stream, mStream.eos, "%f", f);
        return *this;
    }
    TextStreamWriter& TextStreamWriter::operator<<(double const d)
    {
        mStream.stream = gStringFormatAppend(mStream.stream, mStream.eos, "%f", d);
        return *this;
    }

    StringBuilder::StringBuilder(Allocator* allocator, const int capacity)
        : mAllocator(allocator)
        , mBuffer(0)
    {
        grow(capacity);
    }

    StringBuilder::~StringBuilder() { mAllocator->Deallocate(mBuffer); }

    char const* StringBuilder::getText() const { return mBuffer; }

    void StringBuilder::ensure(int bytesRequired)
    {
        if ((mWriter.mStream.stream + bytesRequired + 1) >= mWriter.mStream.eos)
        {
            int const requiredCapacity = bytesRequired + (mWriter.mStream.capacity()) + GROW_CHUNK_SIZE;
            grow(requiredCapacity);
        }
    }

    StringBuilder& StringBuilder::operator<<(const char* txt)
    {
        int const txtLength = gStringLength(txt);
        ensure(txtLength);
        mWriter << txt;
        return *this;
    }
    StringBuilder& StringBuilder::operator<<(const void* p)
    {
        ensure(32);
        mWriter << p;
        return *this;
    }
    StringBuilder& StringBuilder::operator<<(char const n)
    {
        ensure(32);
        mWriter << n;
        return *this;
    }

    StringBuilder& StringBuilder::operator<<(short const n)
    {
        ensure(32);
        mWriter << n;
        return *this;
    }
    StringBuilder& StringBuilder::operator<<(int const n)
    {
        ensure(32);
        mWriter << n;
        return *this;
    }
    StringBuilder& StringBuilder::operator<<(long long const n)
    {
        ensure(32);
        mWriter << n;
        return *this;
    }
    StringBuilder& StringBuilder::operator<<(long const n)
    {
        ensure(32);
        mWriter << n;
        return *this;
    }
    StringBuilder& StringBuilder::operator<<(unsigned char const n)
    {
        ensure(32);
        mWriter << n;
        return *this;
    }
    StringBuilder& StringBuilder::operator<<(unsigned short const n)
    {
        ensure(32);
        mWriter << n;
        return *this;
    }
    StringBuilder& StringBuilder::operator<<(unsigned int const n)
    {
        ensure(32);
        mWriter << n;
        return *this;
    }
    StringBuilder& StringBuilder::operator<<(unsigned long long const n)
    {
        ensure(32);
        mWriter << n;
        return *this;
    }
    StringBuilder& StringBuilder::operator<<(unsigned long const n)
    {
        ensure(32);
        mWriter << n;
        return *this;
    }
    StringBuilder& StringBuilder::operator<<(float const f)
    {
        ensure(32);
        mWriter << f;
        return *this;
    }
    StringBuilder& StringBuilder::operator<<(double const d)
    {
        ensure(64);
        mWriter << d;
        return *this;
    }

    int StringBuilder::getCapacity() const { return mWriter.mStream.capacity(); }

    void StringBuilder::grow(int desiredCapacity)
    {
        int const newCapacity = gRoundUpToMultipleOfPow2Number(desiredCapacity, 32);

        char* buffer = nullptr;
        buffer       = (char*)mAllocator->Allocate(newCapacity, 4);
        if (mBuffer)
            gStringCopy(buffer, mBuffer, mWriter.mStream.used() + 1);
        else
            gStringCopy(buffer, "", 2);

        if (mBuffer != 0)
        {
            mAllocator->Deallocate(mBuffer);
        }

        mBuffer = buffer;

        mWriter.mStream.sos    = mBuffer;
        mWriter.mStream.eos    = mBuffer + newCapacity;
        mWriter.mStream.stream = mBuffer + mWriter.mStream.used();
    }

} // namespace BenchMark
