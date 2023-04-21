#ifndef __CBENCHMARK_STRINGBUILDER_H__
#define __CBENCHMARK_STRINGBUILDER_H__

namespace BenchMark
{
    class Allocator;

    struct TextStream
    {
        inline bool isEof() const { return stream >= eos; }
        inline int  getN() const { return (int)(eos - stream); }
        inline int  used() const { return (int)(stream - sos); }
        inline int  capacity() const { return (int)(eos - sos); }

        char*       stream; // stream cursor
        char*       sos;    // start of stream
        const char* eos;    // end of stream
    };

    class TextStreamWriter
    {
    public:
        TextStream mStream;

        TextStreamWriter& operator<<(const char* txt);
        TextStreamWriter& operator<<(const void* p);
        TextStreamWriter& operator<<(char const n);
        TextStreamWriter& operator<<(short const n);
        TextStreamWriter& operator<<(int const n);
        TextStreamWriter& operator<<(long long const n);
        TextStreamWriter& operator<<(long const n);
        TextStreamWriter& operator<<(unsigned char const n);
        TextStreamWriter& operator<<(unsigned short const n);
        TextStreamWriter& operator<<(unsigned int const n);
        TextStreamWriter& operator<<(unsigned long long const n);
        TextStreamWriter& operator<<(unsigned long const n);
        TextStreamWriter& operator<<(float const f);
        TextStreamWriter& operator<<(double const f);
    };

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
        StringBuilder& operator<<(double const d);

        enum ESettings
        {
            STATIC_CHUNK_SIZE = 512 - 24,
            GROW_CHUNK_SIZE   = 32
        };
        int getCapacity() const;

    private:
        void ensure(int required);
        void grow(int capacity);

        Allocator*       mAllocator;
        char*            mBuffer;
        TextStreamWriter mWriter;
    };
} // namespace BenchMark

#endif ///< __CBENCHMARK_STRINGBUILDER_H__
