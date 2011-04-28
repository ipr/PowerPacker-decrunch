////////////////////////////////////////////////
// PowerPacker.h : interface and utility classes
//
// CPowerPacker : actual unpacking of data
//
// CAnsiFile : simple helper added for file handling
//
// PPException : exception-type
//
// Builds and works at least on VC++ 2010..
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

#ifndef _POWERPACKER_H_
#define _POWERPACKER_H_

#include <string>
#include <exception>

// exception-class for error cases
class PPException : public std::exception
{
public:
	PPException(void)
		: std::exception()
	{
	}

	PPException(const char *szMessage)
		: std::exception(szMessage)
	{
	}
};

// ANSI-C style file-API helper
class CAnsiFile
{
protected:
	FILE *m_pFile;
	size_t m_nFileSize;
	bool m_bIsWritable;

	size_t GetSizeOfFile()
	{
		if (m_pFile == NULL)
		{
			return 0;
		}

		// should be at start (0)
		long lCurrent = ftell(m_pFile);

		// locate end
		if (fseek(m_pFile, 0L, SEEK_END) != 0)
		{
			return 0;
		}
		size_t nSize = ftell(m_pFile);

		// return to start
		fseek(m_pFile, lCurrent, SEEK_SET);
		return nSize;
	}

public:
	CAnsiFile(void)
		: m_pFile(NULL)
		, m_nFileSize(0)
		, m_bIsWritable(false)
	{}
	CAnsiFile(const char *szFile, bool bWritable = false)
		: m_pFile(NULL)
		, m_nFileSize(0)
		, m_bIsWritable(bWritable)
	{
		Open(szFile, bWritable);
	}
	~CAnsiFile(void)
	{
		Close();
	}

	bool Open(const char *szFile, bool bWritable = false)
	{
		Close(); // close previous (if any)

		if (bWritable == false)
		{
			m_pFile = fopen(szFile, "rb");
			m_nFileSize = GetSizeOfFile();
		}
		else
		{
			// size zero if new file..
			m_pFile = fopen(szFile, "wb");
		}
		m_bIsWritable = bWritable;
		return IsOk();
	}

	void Close()
	{
		if (m_pFile != NULL)
		{
			fclose(m_pFile);
			m_pFile = NULL;
		}
	}

	size_t GetSize()
	{
		// get size as it was in start
		if (m_bIsWritable == false)
		{
			return m_nFileSize;
		}
		// determine current size
		return GetSizeOfFile();
	}

	bool IsOk()
	{
		if (m_pFile == NULL)
		{
			return false;
		}

		if (ferror(m_pFile) != 0) 
		{
			return false;
		}
		return true;
	}

	bool Flush()
	{
		if (fflush(m_pFile) != 0)
		{
			return false;
		}
		return true;
	}

	bool Write(const void *pBuffer, const size_t nBytes)
	{
		size_t nWritten = fwrite(pBuffer, 1, nBytes, m_pFile);
		if (IsOk() == false
			|| nWritten < nBytes)
		{
			return false;
		}
		return Flush();
	}

	bool Read(void *pBuffer, const size_t nBytes)
	{
		size_t nRead = fread(pBuffer, 1, nBytes, m_pFile);
		if (IsOk() == false
			|| nRead < nBytes)
		{
			return false;
		}
		return true;
	}
};

// typedefs (note: check sizes..)
typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

// buffer descriptor for in/out buffers
struct tBuffer 
{
	tBuffer()
	{
		ptr = NULL;
		size = 0;

		pos = NULL;
		pos_end = NULL;
	};

	uchar *ptr; // buffer of data
	ulong size; // size of buffer

	// position pointers for decompression
	uchar *pos;
	uchar *pos_end;
};

// metadata used during uncompressing
struct tMetaBits
{
	tMetaBits()
	{
		ptrbit_2 = 0;
		ptrbit_3 = 0;
		ptrbit_4 = 0;
		ptrbit_5 = 0;
		bitrot = 0;
	};

	uint ptrbit_2;
	uint ptrbit_3;
	uint ptrbit_4;
	uint ptrbit_5;
	uint bitrot;

	// note: there are five levels of compression supported,
	// these check that file has one of those and that it can be unpacked:
	// - $09090909 - Efficiency Fast
	// - $090A0A0A - Efficiency Mediocre
	// - $090A0B0B - Efficiency Good
	// - $090A0C0C - Efficiency Very Good
	// - $090A0C0D - Efficiency Best
	//
	bool IsMetadataOk()
	{
		if(!(
				(ptrbit_2 == 9 && ptrbit_3 == 9 && ptrbit_4 == 9 && ptrbit_5 == 9) ||
				(ptrbit_2 == 9 && ptrbit_3 == 10 && ptrbit_4 == 10 && ptrbit_5 == 10) ||
				(ptrbit_2 == 9 && ptrbit_3 == 10 && ptrbit_4 == 11 && ptrbit_5 == 11) ||
				(ptrbit_2 == 9 && ptrbit_3 == 10 && ptrbit_4 == 12 && ptrbit_5 == 12) ||
				(ptrbit_2 == 9 && ptrbit_3 == 10 && ptrbit_4 == 12 && ptrbit_5 == 13)
			) || bitrot >= 32)
		{
			return false;
		}
		return true;
	};
};

// metadata in file header
struct tHeaderMetaData
{
	tHeaderMetaData()
	{
		::memset(meta, 0, 14);
	};

	uchar meta[14];

	// check file type ID if is supported file type
	bool IsSupportedFiletype()
	{
		// "PX20" -> encrypted file
		if (meta[0] == 80 && meta[1] == 88 && meta[2] == 50 && meta[3] == 48)
		{
			return false;
		}

		// not "PP20" -> not Powerpacker file
		// (note: there's also "PP11" for version 1.1 ?)
		if (!(meta[0] == 80 && meta[1] == 80 && meta[2] == 50 && meta[3] == 48))
		{
			return false;
		}
		return true;
	}

	// get uncompressed size of file for buffer allocation
	ulong GetUncompressedSize()
	{
		return (meta[10] << 16 | meta[11] << 8 | meta[12]);
	}

	// copy some values used during decompression
	// (may be modified)
	void CopyMetaBits(tMetaBits &MetaBits)
	{
		MetaBits.ptrbit_2 = meta[6]; 
		MetaBits.ptrbit_3 = meta[7]; 
		MetaBits.ptrbit_4 = meta[8]; 
		MetaBits.ptrbit_5 = meta[9]; 
		MetaBits.bitrot = meta[13];
	}
};

// 
class CPowerPacker
{
protected:

	// buffer descriptors for input and output buffers
	struct tBuffer m_in_list;
	struct tBuffer m_out_list;

	// metadata of file
	struct tHeaderMetaData m_Meta;
	struct tMetaBits m_MetaBits;

	// table for reversing bits in a byte
	uchar m_rev_table[256];
	char m_rev_build;

	/* build a table to reverse the bits in a byte */
	void rev_init()
	{
		if (m_rev_build != 0) 
		{
			return;
		}

		for (uint a = 0; a < 256; a++) 
		{
			uchar bit = a;
			bit = (bit & 0x0f) << 4 | (bit >> 4 & 0x0f);
			bit = (bit & 0x33) << 2 | (bit >> 2 & 0x33);
			bit = (bit & 0x55) << 1 | (bit >> 1 & 0x55);
			m_rev_table[a] = bit;
		}
		m_rev_build = 1;
	};

	bool Peek(uint x, tBuffer &in_list, ulong &code, uint &shift) const
	{
		while (shift > 32 - (x)) 
		{
			if (in_list.pos == in_list.pos_end)
			{
				//"end of input"
				return false;
			}
			shift -= 8; 
			code += (m_rev_table[*--in_list.pos] << shift); 
		}
		return true;
	};

	void Shift(uint x, ulong &code, uint &shift) const
	{
		shift += (x); 
		code <<= (x); 
		code &= 0xffffffff;
	};

	void BitSwitch(uint &bit, uint &len, ulong &code, uint &shift) const
	{
		bit = code >> 29;
		switch (bit) 
		{
		case 2: 
		case 3: 
			Shift(2, code, shift); len = 3; bit = m_MetaBits.ptrbit_3; 
			break;

		case 4: 
		case 5: 
			Shift(2, code, shift); len = 4; bit = m_MetaBits.ptrbit_4; 
			break;

		case 6: 
			Shift(3, code, shift); len = 5; bit = 7; 
			break;

		case 7: 
			Shift(3, code, shift); len = 5; bit = m_MetaBits.ptrbit_5; 
			break;

		default: 
			Shift(2, code, shift); len = 2; bit = m_MetaBits.ptrbit_2; 
			break;
		}
	};

	void Clean()
	{
		if (m_in_list.ptr) 
		{
			free(m_in_list.ptr);
			m_in_list.ptr = 0;
		}
		if (m_out_list.ptr) 
		{
			free(m_out_list.ptr);
			m_out_list.ptr = 0;
		}
	}

	// decompression, unpacking
	void UnPowerpack();

	void LoadBuffer(const uchar *pData, const ulong nSize);
	void Load(const char *szInputFile);
	void Save(const char *szOutputFile);
	void CheckExisting(const char *szOutputFile);

public:
	CPowerPacker(void)
		: m_rev_build(0)
		, m_in_list()
		, m_out_list()
		, m_Meta()
		, m_MetaBits()
	{
		rev_init();
	}

	~CPowerPacker(void)
	{
		Clean();
	}

	// TODO: allow unpacking already read file from a buffer
	/*
	bool UnpackBuffer(const uchar *pData, const ulong nSize)
	{
		LoadBuffer(pData, nSize);
		UnPowerpack();
		return true;
	}
	*/

	// load packed file and unpack to memory
	bool UnpackFile(const char *szInputFile)
	{
		Load(szInputFile);
		UnPowerpack();
		return true;
	}

	// save unpacked data to specified file
	bool SaveToFile(const char *szOutputFile)
	{
		CheckExisting(szOutputFile);
		Save(szOutputFile);
		return true;
	}

	// access unpacked data
	uchar *GetUnpackedData()
	{
		return m_out_list.ptr;
	}
	ulong GetUnpackedSize()
	{
		return m_out_list.size;
	}

};

#endif // ifndef _POWERPACKER_H_


