
PowerPacker decompression in simple reusable C++ classes.

Author: Ilkka Prusi
Contact: ilkka.prusi@gmail.com

Based on PP-Tools in Aminet:
Author: david tritscher
Uploader: davidtritscher@dodo.com.au

Summary:
Decrunch/decompress PowerPacker-compressed files.

Purpose:
Simple C++ class(es) for reusable code to decrunch those files.

Description:
PowerPacker was popular compression for files on Commodore Amiga computers.
It was used for sound modules, executables, game data files etc.

Actual code is in three classes:
- CPowerPacker : actual unpacking of data
- CAnsiFile : simple helper added for file handling
- PPException : exception-type

All necessary code is in two files:
PowerPacker.cpp and PowerPacker.h

Sample-app in file: ppack.cpp

Code has unlimited distribution, same as original PP-Tools.
