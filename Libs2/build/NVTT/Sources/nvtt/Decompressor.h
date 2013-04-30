// OTHER DEALINGS IN THE SOFTWARE.

#ifndef NV_TT_DECOMPRESSOR_H
#define NV_TT_DECOMPRESSOR_H

#include <nvcore/Ptr.h>

#include "nvtt_extra.h"
#include <nvimage/DirectDrawSurface.h>

namespace nvtt
{
	struct Decompressor::Private
	{
		Private() 
		{
			m_dds = NULL;
			m_memmoryBufferPointer = NULL;
		}

		bool initWithDDSFile(const char * pathToDDSFile);
		
		bool initWithDDSFile(FILE *file);

		bool initWithDDSFile(const uint8 * mem, uint size);
		
		void erase();

		bool decompress(void * data, unsigned int size, unsigned int mipmapNumber) const;

		bool getInfo(unsigned int & mipmapCount, unsigned int & width, unsigned int & heigth, unsigned int & size, unsigned int & headerSize) const;

		bool getCompressionFormat(Format & comprFormat) const;

		bool getRawData(void* buffer, unsigned int size) const;

		bool getMipmapSize(unsigned int number, unsigned int & size) const;

		static unsigned int getHeader(void* buffer, const unsigned int bufferSize, const InputOptions::Private & inputOptions, const CompressionOptions::Private & compressionOptions);

		private:

		nv::DirectDrawSurface * m_dds;

		uint8				  * m_memmoryBufferPointer;

	};

} // nvtt namespace


#endif // NV_TT_DECOMPRESSOR_H