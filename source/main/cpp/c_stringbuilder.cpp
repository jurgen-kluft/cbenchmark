#include "cbenchmark/private/c_config.h"
#include "cbenchmark/private/c_stringbuilder.h"
#include "cbenchmark/private/c_utils.h"

namespace BenchMark
{
	StringBuilder::StringBuilder(BenchMarkAllocator* allocator, const int capacity)
		: mAllocator(allocator)
		, mCapacity(0)
		, mCursor(0)
		, mBuffer(0)
	{
		mDefaultBuffer[0] = '\0';
		growBuffer(capacity);
	}

	StringBuilder::~StringBuilder()
	{
		if (mBuffer != mDefaultBuffer)
			mAllocator->Deallocate(mBuffer);
	}

	char const* StringBuilder::getText() const
	{
		return mBuffer;
	}

	char* StringBuilder::getWriteBuffer(int bytesRequired)
	{
		int const streamLength = mCursor + 1;
		int const bytesLeft = mCapacity - streamLength;
		if ((bytesRequired+1) > bytesLeft)
		{
			int const requiredCapacity = bytesRequired + mCapacity - bytesLeft + GROW_CHUNK_SIZE;
			growBuffer(requiredCapacity);
		}

		return mBuffer + mCursor;
	}

	StringBuilder& StringBuilder::operator << (const char* txt)
	{
		int const streamLength = mCursor + 1;
		int const txtLength = gStringLength(txt);

		int const bytesLeft = mCapacity - streamLength;
		int const bytesRequired = (int)txtLength + 1;

		if (bytesRequired > bytesLeft)
		{
			int const requiredCapacity = bytesRequired + mCapacity - bytesLeft + GROW_CHUNK_SIZE;
			growBuffer(requiredCapacity);
		}

		for (int i=0; i<(txtLength+1); i++)
		{
			mBuffer[streamLength + i] = txt[i];
		}

		mCursor += txtLength;

		return *this;
	}
	StringBuilder& StringBuilder::operator << (const void* p)
	{
		char* dest = getWriteBuffer(32);
		gStringPrint(dest, sizeof(dest), "%0X", (size_t)p);
		return *this;    
	}
	StringBuilder& StringBuilder::operator << (char const n)
	{
		char* dest = getWriteBuffer(32);
		gStringPrint(dest, sizeof(dest), "%i", n);
		return *this;
	}

	StringBuilder& StringBuilder::operator << (short const n)
	{
		char* dest = getWriteBuffer(32);
		gStringPrint(dest, sizeof(dest), "%i", n);
		return *this;
	}
	StringBuilder& StringBuilder::operator << (int const n)
	{
		char* dest = getWriteBuffer(32);
		gStringPrint(dest, sizeof(dest), "%i", n);
		return *this;
	}
	StringBuilder& StringBuilder::operator << (long long const n)
	{
		char* dest = getWriteBuffer(32);
		gStringPrint(dest, sizeof(dest), "%i", n);
		return *this;
	}
	StringBuilder& StringBuilder::operator << (long const n)
	{
		char* dest = getWriteBuffer(32);
		gStringPrint(dest, sizeof(dest), "%i", n);
		return *this;
	}
	StringBuilder& StringBuilder::operator << (unsigned char const n)
	{
		char* dest = getWriteBuffer(32);
		gStringPrint(dest, sizeof(dest), "%i", n);
		return *this;
	}
	StringBuilder& StringBuilder::operator << (unsigned short const n)
	{
		char* dest = getWriteBuffer(32);
		gStringPrint(dest, sizeof(dest), "%i", n);
		return *this;
	}
	StringBuilder& StringBuilder::operator << (unsigned int const n)
	{
		char* dest = getWriteBuffer(32);
		gStringPrint(dest, sizeof(dest), "%i", n);
		return *this;
	}
	StringBuilder& StringBuilder::operator << (unsigned long long const n)
	{
		char* dest = getWriteBuffer(32);
		gStringPrint(dest, sizeof(dest), "%i", n);
		return *this;
	}
	StringBuilder& StringBuilder::operator << (unsigned long const n)
	{
		char* dest = getWriteBuffer(32);
		gStringPrint(dest, sizeof(dest), "%i", n);
		return *this;
	}
	StringBuilder& StringBuilder::operator << (float const f)
	{
		char* dest = getWriteBuffer(32);
		gStringPrint(dest, sizeof(dest), "%f", f);
		return *this;    
	}
	StringBuilder& StringBuilder::operator << (double const d)
	{
		char* dest = getWriteBuffer(64);
		float f = (float)d;
		gStringPrint(dest, sizeof(dest), "%f", f);
		return *this;    
	}

	int StringBuilder::getCapacity() const
	{
		return mCapacity;
	}

	void StringBuilder::growBuffer(int desiredCapacity)
	{
		int const newCapacity = gRoundUpToMultipleOfPow2Number(desiredCapacity, 32);

        char*     buffer      = nullptr;
        if (newCapacity <= STATIC_CHUNK_SIZE)
		{
			buffer = mDefaultBuffer;
			if (mBuffer)
			{
				if (mDefaultBuffer != mBuffer)
					gStringCopy(buffer, mBuffer, mCapacity);
			}
			else
			{
				gStringCopy(buffer, "", mCapacity);
			}
		}
		else 
		{
            buffer = (char*)mAllocator->Allocate(newCapacity, 4);
            if (mBuffer)
                gStringCopy(buffer, mBuffer, mCursor + 1);
            else
                gStringCopy(buffer, "", mCapacity);
        }

		if (mBuffer != mDefaultBuffer && mBuffer!=0)
		{
			mAllocator->Deallocate(mBuffer);
		}

		mBuffer = buffer;
		mCapacity = newCapacity;
	}

}
