#pragma once

#include "zipper.h"
using namespace::ziputils;

class ZlibUtil
{
public:

	static bool CreateZipFile( const char* zipPath, std::string jsonPath )
	{
		const unsigned int BUFSIZE = 2048;

		//zipper zipFile;
		zipFile	zipFile_ = 0;
		bool	entryOpen_ = false;

		//zipFile.create( zipPath );
		zipFile_ = zipOpen64( zipPath, APPEND_STATUS_CREATE );

		//// Add file into a folder
		ifstream file( jsonPath, ios::in | ios::binary );
		if ( file.is_open() )
		{
			const char* str = "Capture.json";

			//zipFile.addEntry( str.c_str() );
			if ( zipFile_ != 0 )
			{
				if ( entryOpen_ )
				{
					zipCloseFileInZip( zipFile_ );
					entryOpen_ = false;
				}

				while ( str[0] == '\\' || str[0] == '/' )
					str++;

				//?? we dont need the stinking time
				zip_fileinfo zi = { 0 };
				//getTime( zi.tmz_date );
				tm_zip& tmZip = zi.tmz_date;
				time_t rawtime;
				time(&rawtime);
				auto timeinfo = localtime(&rawtime);
				tmZip.tm_sec = timeinfo->tm_sec;
				tmZip.tm_min = timeinfo->tm_min;
				tmZip.tm_hour = timeinfo->tm_hour;
				tmZip.tm_mday = timeinfo->tm_mday;
				tmZip.tm_mon = timeinfo->tm_mon;
				tmZip.tm_year = timeinfo->tm_year;

				int err = zipOpenNewFileInZip(zipFile_, str, &zi,
					NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);

				entryOpen_ = (err == ZIP_OK);
			}

			//zipFile << file;
			int err = ZIP_OK;
			char buf[BUFSIZE];
			unsigned long nRead = 0;

			if (entryOpen_)
			{
				while (err == ZIP_OK && file.good())
				{
					file.read(buf, BUFSIZE);
					unsigned int nRead = (unsigned int)file.gcount();

					if (nRead)
					{
						err = zipWriteInFileInZip( zipFile_, buf, nRead );
					}
					else
					{
						break;
					}
				}
			}
			//return *this;
		}
		file.close();

		if ( std::remove( jsonPath.c_str() ) != 0 )
			perror( "Error deleting file" );	// Not working in unreal.

		//zipFile.close();
		if ( zipFile_ )
		{
			if ( entryOpen_ )
			{
				zipCloseFileInZip( zipFile_ );
				entryOpen_ = false;
			}

			zipClose( zipFile_, 0 );
			zipFile_ = 0;
		}

		return true;
	}
};
