#ifndef __CBENCHMARK_STRINGBUILDER_H__
#define __CBENCHMARK_STRINGBUILDER_H__

namespace BenchMark
{
    class Allocator;

    class StringBuilder
    {
    public:
        StringBuilder(Allocator* allocator, const int capacity = 480);
        ~StringBuilder();

        char const* getText() const;

        StringBuilder& operator<<(const char* txt);
        StringBuilder& operator<<(const void* p);
        StringBuilder& operator<<(char const n);
        StringBuilder& operator<<(short const n);
        StringBuilder& operator<<(int const n);
        StringBuilder& operator<<(long long const n);
        StringBuilder& operator<<(long const n);
        StringBuilder& operator<<(unsigned char const n);
        StringBuilder& operator<<(unsigned short const n);
        StringBuilder& operator<<(unsigned int const n);
        StringBuilder& operator<<(unsigned long long const n);
        StringBuilder& operator<<(unsigned long const n);
        StringBuilder& operator<<(float const f);
        StringBuilder& operator<<(double const f);

        enum ESettings
        {
            STATIC_CHUNK_SIZE = 512 - 24,
            GROW_CHUNK_SIZE   = 32
        };
        int getCapacity() const;

    private:
        char* getWriteBuffer(int bytesRequired);
        void  growBuffer(int capacity);

        char                mDefaultBuffer[STATIC_CHUNK_SIZE];
        Allocator* mAllocator;
        int                 mCapacity;
        int                 mCursor;
        char*               mBuffer;
    };
} // namespace BenchMark

#endif ///< __CBENCHMARK_STRINGBUILDER_H__
