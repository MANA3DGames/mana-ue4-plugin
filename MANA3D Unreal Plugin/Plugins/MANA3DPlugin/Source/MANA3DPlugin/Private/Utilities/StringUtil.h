#pragma once

class StringUtil
{
public:
	static char* TCHAR_to_charPointer( const TCHAR* tchar )
	{
		int size = 0;
		while ( (char)tchar[size] != '\0' )
			size++;
		size++;

		char* charpointer = new char[size];
		wcstombs( charpointer, tchar, size );
		return charpointer;
	}
};