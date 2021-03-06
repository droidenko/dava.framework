/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __LOGENGINE_MEMORYFILE_H__
#define __LOGENGINE_MEMORYFILE_H__


#include "BaseTypes.h"
#include "IBase.h"
#include "IFile.h"
#include "StringFormat.h"

namespace Log
{
namespace IO
{


class MemoryFile : public IFile
{
public:
	MemoryFile(uint8 * data, uint32 dataSize, uint32 attributes);
	MemoryFile(uint32 dataSize, uint32 attributes);
	virtual ~MemoryFile();
	

	//! \brief for internal engine usage
	void * GetPointer() { return (void *)dataStart; }

	//! \brief Get this file name
	virtual	const char8 *		GetFilename();
    
	//! \brief Get this file full pathname
	virtual const char8 *		GetPathname();
	
	//! \brief Write [dataSize] bytes to this file from [pointerToData]
	//! \param pointerToData function get data from this pointer
	//! \param dataSize size of data we want to write
	//! \return number of bytes actually written
	virtual uint32				Write(void * pointerToData, uint32 dataSize);
	
	//! \brief Read [dataSize] bytes from this file to [pointerToData] 
	//! \param pointerToData function write data to this pointer
	//! \param dataSize size of data we want to read
	//! \return number of bytes actually read
	virtual uint32				Read(void * pointerToData, uint32 dataSize);

	//! \brief Get current file position
	virtual uint32				GetPos();

	//! \brief Get current file size if writing
	//! \brief and get real file size if file for reading
	virtual	uint32				GetSize();

	//! \brief Set current file position
	//! \param position position to set
	//! \param seekType \ref IO::eFileSeek flag to set type of positioning
	//! \return true if successful otherwise false.
	virtual bool				Seek(uint32 position, uint32 seekType);


	//! return true if end of file reached and false in another case
	virtual bool				IsEof();
private:
	bool	releaseBuffer;

	uint8	* dataStart;
	uint8	* dataCurrent;
	uint32	fileSize;
	uint32  attributes;

	String	memoryFilename;
};


};
};

#endif // __LOGENGINE_MEMORYFILE_H__



