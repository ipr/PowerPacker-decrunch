/////////////////////////////////////////////////////////
// ppack.cpp : sample app using the decruncher.
//
// Ilkka Prusi, 2011
// ilkka.prusi@gmail.com
//
// Unlimited distribution.
//

//#include "stdafx.h"
#include "PowerPacker.h"

#include <iostream>


int _tmain(int argc, _TCHAR* argv[])
{
	// at least input-file expected
	if (argc < 2)
	{
		return 1;
	}

	// should throw exception on critical error
	try
	{
		CPowerPacker PP;

		if (argc < 3)
		{
			// just unpack to RAM, no file output
			PP.UnpackFile(argv[1]);
		}
		else
		{
			// unpack to RAM and output to specified (new) file
			PP.UnpackFile(argv[1]);
			PP.SaveToFile(argv[2]);
		}

		// handle data before destroyed 
		// (when no writing to file):
		// for example, show pic or play module
		//
		uchar *pData = PP.GetUnpackedData();
		ulong ulSize = PP.GetUnpackedSize();
	}
	catch (PPException &exp)
	{
		std::cout << exp.what() << std::endl;
	}

	return 0;
}

