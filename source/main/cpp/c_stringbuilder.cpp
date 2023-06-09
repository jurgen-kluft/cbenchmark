#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_stringbuilder.h"
#include "cbenchmark/private/c_utils.h"
#include "cbenchmark/private/c_types.h"
#include "cbenchmark/private/c_benchmark_allocators.h"

namespace BenchMark
{
    // TODO Windows EOL, what about other platforms ?
    TextStream& TextStream::endl() 
    { 
        flush();
        out->print("\n");
        return *this;
    }

    TextStream& TextStream::operator<<(const char* txt)
    {
        flush();
        out->print(txt);
        return *this;
    }
    TextStream& TextStream::operator<<(const void* p)
    {
        stream = gStringFormatAppend(stream, eos, "%0X", (u64)p);
        return flush();
    }
    TextStream& TextStream::operator<<(char const n)
    {
        stream = gStringFormatAppend(stream, eos, "%i", n);
        return flush();
    }
    TextStream& TextStream::operator<<(short const n)
    {
        stream = gStringFormatAppend(stream, eos, "%i", n);
        return flush();
    }
    TextStream& TextStream::operator<<(int const n)
    {
        stream = gStringFormatAppend(stream, eos, "%i", n);
        return flush();
    }
    TextStream& TextStream::operator<<(long long const n)
    {
        stream = gStringFormatAppend(stream, eos, "%i", n);
        return flush();
    }
    TextStream& TextStream::operator<<(long const n)
    {
        stream = gStringFormatAppend(stream, eos, "%i", n);
        return flush();
    }
    TextStream& TextStream::operator<<(unsigned char const n)
    {
        stream = gStringFormatAppend(stream, eos, "%i", n);
        return flush();
    }
    TextStream& TextStream::operator<<(unsigned short const n)
    {
        stream = gStringFormatAppend(stream, eos, "%i", n);
        return flush();
    }
    TextStream& TextStream::operator<<(unsigned int const n)
    {
        stream = gStringFormatAppend(stream, eos, "%i", n);
        return flush();
    }
    TextStream& TextStream::operator<<(unsigned long long const n)
    {
        stream = gStringFormatAppend(stream, eos, "%i", n);
        return flush();
    }
    TextStream& TextStream::operator<<(unsigned long const n)
    {
        stream = gStringFormatAppend(stream, eos, "%i", n);
        return flush();
    }
    TextStream& TextStream::operator<<(float const f)
    {
        stream = gStringFormatAppend(stream, eos, "%f", f);
        return flush();
    }
    TextStream& TextStream::operator<<(double const d)
    {
        stream = gStringFormatAppend(stream, eos, "%f", d);
        return flush();
    }
    TextStream& TextStream::flush()
    {
        *stream++ = '\0';
        if (stream >= eos)
        {
            out->print(sos);
            stream = sos;
        }
        return *this;
    }

    TextStream& TextStream::operator<<(TextColor const color)
    { 
        flush();
        if (color == COLOR_DEFAULT)
        {
            out->resetColor();
            return *this;
        }
        out->setColor(color);
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
        if ((mWriter.stream + bytesRequired + 1) >= mWriter.eos)
        {
            int const requiredCapacity = bytesRequired + (mWriter.capacity()) + GROW_CHUNK_SIZE;
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

    int StringBuilder::getCapacity() const { return mWriter.capacity(); }

    void StringBuilder::grow(int desiredCapacity)
    {
        int const newCapacity = gRoundUpToMultipleOfPow2Number(desiredCapacity, 32);

        char* buffer = nullptr;
        buffer       = (char*)mAllocator->Allocate(newCapacity, 4);
        if (mBuffer)
            gStringCopy(buffer, mBuffer, mWriter.used() + 1);
        else
            gStringCopy(buffer, "", 2);

        if (mBuffer != 0)
        {
            mAllocator->Deallocate(mBuffer);
        }

        mBuffer = buffer;

        mWriter.sos    = mBuffer;
        mWriter.eos    = mBuffer + newCapacity;
        mWriter.stream = mBuffer + mWriter.used();
    }

} // namespace BenchMark
