////////////////////////////////////////////////
// PowerPacker.cpp
//
// See details of classes in header-file.
//
// Based on PP-Tools in Aminet:
// Author: david tritscher
// Uploader: davidtritscher@dodo.com.au
//
// Modified to C++ by Ilkka Prusi, 2011
// ilkka.prusi@gmail.com
//
// Unlimited distribution.
//

//#include "stdafx.h"
#include "PowerPacker.h"


void CPowerPacker::UnPowerpack()
{
	uint bit = 0;
	uint len = 0;
	uint ptr = 0;

	ulong code = 0;
	uint shift = 32;

	// pointers for processing
	m_in_list.pos = m_in_list.ptr + m_in_list.size; 
	m_in_list.pos_end = m_in_list.ptr;
	m_out_list.pos = m_out_list.ptr + m_out_list.size; 
	m_out_list.pos_end = m_out_list.ptr;

	if (Peek(m_MetaBits.bitrot, m_in_list, code, shift) == false)
	{
		return;
	}
	Shift(m_MetaBits.bitrot, code, shift);

	if (Peek(1, m_in_list, code, shift) == false)
	{
		return;
	}

	uint protect = code >> 31;
	for(;;) 
	{
		if (Peek(3, m_in_list, code, shift) == false)
		{
			return;
		}

		bit = code >> 31; 
		Shift(1, code, shift);
		if(bit == protect) 
		{
			len = (code >> 30) + 1; 
			Shift(2, code, shift);
			if(len == 4) 
			{
				do 
				{
					if (Peek(2, m_in_list, code, shift) == false)
					{
						return;
					}
					bit = code >> 30; 
					Shift(2, code, shift);
					len += bit;
				} while(bit == 3);
			}

			if (len > m_out_list.pos - m_out_list.pos_end) 
			{
				throw PPException("bad character length");
			}

			while(len > 0) 
			{
				/* copy characters */
				if (Peek(8, m_in_list, code, shift) == false)
				{
					return;
				}
				*--m_out_list.pos = code >> 24; 
				Shift(8, code, shift); 
				len--;
			}

			if (m_out_list.pos == m_out_list.pos_end) 
			{
				break;
			}
		}

		if (Peek(3, m_in_list, code, shift) == false)
		{
			return;
		}

		BitSwitch(bit, len, code, shift);

		if (Peek(bit, m_in_list, code, shift) == false)
		{
			return;
		}

		ptr = (code >> (32 - bit)) + 1;

		Shift(bit, code, shift);

		if (ptr > m_out_list.pos_end + m_out_list.size - m_out_list.pos) 
		{
			throw PPException("bad string pointer");
		}

		if (len == 5) 
		{
			do 
			{
				if (Peek(3, m_in_list, code, shift) == false)
				{
					return;
				}
				bit = code >> 29; 
				Shift(3, code, shift);
				len += bit;
			} while(bit == 7);
		}

		if (len > m_out_list.pos - m_out_list.pos_end) 
		{
			throw PPException("bad string length");
		}

		uchar *str = m_out_list.pos + ptr;
		while(len > 0) 
		{
			*--m_out_list.pos = *--str; /* copy string */
			len--;
		}

		if (m_out_list.pos == m_out_list.pos_end)
		{
			break;
		}
	}
}

void CPowerPacker::LoadBuffer(const uchar *pData, const ulong nSize)
{
	/*
	::memcpy(m_Meta.meta, pData, 4);

	if (m_Meta.IsSupportedFiletype() == false)
	{
		throw PPException("file type is not supported");
	}

	::memcpy(m_Meta.meta + 6, pData + 4, 4);
	m_in_list.size = (nSize - 8);
	if (m_in_list.size < 8 || m_in_list.size > 0x1000000 || m_in_list.size % 4 != 0) 
	{
		throw PPException("bad compressed size");
	}

	m_in_list.ptr = (uchar*)malloc(m_in_list.size);
	::memcpy(m_in_list.ptr, pData + 8, m_in_list.size);

	m_in_list.size -= 4;
	::memcpy(m_Meta.meta + 10, m_in_list.ptr + m_in_list.size, 4);

	m_Meta.CopyMetaBits(m_MetaBits);
	if (m_MetaBits.IsMetadataOk() == false)
	{
		throw PPException("bad metadata");
	}

	m_out_list.size = m_Meta.GetUncompressedSize();
	if (m_out_list.size == 0) 
	{
		throw PPException("bad uncompressed size");
	}

	m_out_list.ptr = (uchar*)malloc(m_out_list.size);
	if (m_out_list.ptr == 0) 
	{
		throw PPException("failed to allocate memory for output");
	}
	*/
}

void CPowerPacker::Load(const char *szInputFile)
{
	CAnsiFile InputFile(szInputFile, false);
	if (InputFile.IsOk() == false)
	{
		throw PPException("failed to open file");
	}

	if (InputFile.Read(m_Meta.meta, 4) == false)
	{
		throw PPException("failed to read file");
	}

	if (m_Meta.IsSupportedFiletype() == false)
	{
		throw PPException("file type is not supported");
	}

	if (InputFile.Read(m_Meta.meta + 6, 4) == false)
	{
		throw PPException("failed to read file");
	}

	// get size for buffer allocation
	// (decrement what was already read at header)
	m_in_list.size = InputFile.GetSize() - 8;

	// limited to 16MB ?
	// also size must be in power of four?
	if (m_in_list.size < 8 || m_in_list.size > 0x1000000 || m_in_list.size % 4 != 0) 
	{
		throw PPException("bad compressed size");
	}

	// allocate and read at once
	m_in_list.ptr = (uchar*)malloc(m_in_list.size);
	if (InputFile.Read(m_in_list.ptr, m_in_list.size) == false)
	{
		throw PPException("failed to read file");
	}

	// no longer necessary
	InputFile.Close();

	// size is at last bytes?
	m_in_list.size -= 4;
	memcpy(m_Meta.meta + 10, m_in_list.ptr + m_in_list.size, 4);

	/* metadata */
	m_Meta.CopyMetaBits(m_MetaBits);
	if (m_MetaBits.IsMetadataOk() == false)
	{
		throw PPException("bad metadata");
	}

	m_out_list.size = m_Meta.GetUncompressedSize();
	if (m_out_list.size == 0) 
	{
		throw PPException("bad uncompressed size");
	}

	m_out_list.ptr = (uchar*)malloc(m_out_list.size);
	if (m_out_list.ptr == 0) 
	{
		throw PPException("failed to allocate memory for output");
	}
}

void CPowerPacker::Save(const char *szOutputFile)
{
	CAnsiFile OutputFile(szOutputFile, true);
	if (OutputFile.IsOk() == false)
	{
		throw PPException("failed to open file to write");
	}

	if (OutputFile.Write(m_out_list.ptr, m_out_list.size) == false)
	{
		throw PPException("failed to write file");
	}
}

void CPowerPacker::CheckExisting(const char *szOutputFile)
{
	// rename if output exists already
	int iLen = strlen(szOutputFile);
	char *pTmp = (char*)::malloc(iLen+2);
	strcpy(pTmp, szOutputFile);
	strcat(pTmp, "~");

	remove(pTmp);
	rename(szOutputFile, pTmp);

	::free(pTmp);
}

