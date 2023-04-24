#ifndef __CBENCHMARK_STRINGBUILDER_H__
#define __CBENCHMARK_STRINGBUILDER_H__

namespace BenchMark
{
    class Allocator;
    class TextStream;

    enum TextColor
    {
        COLOR_DEFAULT,
        COLOR_RED,
        COLOR_GREEN,
        COLOR_YELLOW,
        COLOR_BLUE,
        COLOR_MAGENTA,
        COLOR_CYAN,
        COLOR_WHITE
    };

    void FormatFloat(TextStream& stream, double value, int decimal_places, int prec);
    void FormatTime(TextStream& stream, double time);
    void FormatString(TextStream& stream, const char* text, int width=0);

    class TextStream
    {
    public:
        inline bool isEof() const { return stream >= eos; }
        inline int  getN() const { return (int)(eos - stream); }
        inline int  used() const { return (int)(stream - sos); }
        inline int  capacity() const { return (int)(eos - sos); }

        TextStream& operator<<(const char* txt);
        TextStream& operator<<(const void* p);
        TextStream& operator<<(char const n);
        TextStream& operator<<(short const n);
        TextStream& operator<<(int const n);
        TextStream& operator<<(long long const n);
        TextStream& operator<<(long const n);
        TextStream& operator<<(unsigned char const n);
        TextStream& operator<<(unsigned short const n);
        TextStream& operator<<(unsigned int const n);
        TextStream& operator<<(unsigned long long const n);
        TextStream& operator<<(unsigned long const n);
        TextStream& operator<<(float const f);
        TextStream& operator<<(double const f);
        TextStream& operator<<(TextColor const color);


        virtual void flush() = 0;

        char*       stream; // stream cursor
        char*       sos;    // start of stream
        const char* eos;    // end of stream
        u64         attrs;  // stream attributes
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

        Allocator* mAllocator;
        char*      mBuffer;
        TextStream mWriter;
    };
} // namespace BenchMark

#endif ///< __CBENCHMARK_STRINGBUILDER_H__
