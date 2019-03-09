#pragma once
#include "MANA3DPluginPch.h"

#include "Windows/WindowsSystemIncludes.h"
#include "AllowWindowsPlatformTypes.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include "HideWindowsPlatformTypes.h"

#include <fstream>
using namespace::std;


// Structure to hold the response data.
struct MemoryStruct 
{
	char *memory;
	size_t size;
};

class MANA3DCurlRequest
{
private:
	// Responsible for writing the response of the http request to the memory.
	static size_t WriteMemoryCallback( void *ptr, size_t size, size_t nmemb, void *context )
	{
		size_t bytec = size * nmemb;
		struct MemoryStruct *mem = (struct MemoryStruct *)context;
		mem->memory = (char *)realloc(mem->memory, mem->size + bytec + 1);
		if (mem->memory == NULL) 
		{
			GLog->Log( "not enough memory (realloc returned NULL)\n" );
			return 0;
		}
		memcpy(&(mem->memory[mem->size]), ptr, bytec);
		mem->size += bytec;
		mem->memory[mem->size] = 0;
		return nmemb;
	}

public:
	static void SubmitFile( const char* api_key, const char* api_secret, const char* file )
	{
		GLog->Log( "Submitting..." );

		CURL *curl;
		CURLcode res;

		// Memory to save the response.
		struct MemoryStruct chunk;
		chunk.memory = (char *)malloc(1);
		chunk.size = 0;
		chunk.memory[chunk.size] = 0;

		// HTTP form.
		struct curl_httppost *formpost = NULL;
		struct curl_httppost *lastptr = NULL;

		curl_global_init( CURL_GLOBAL_ALL );

		// Set AppID.
		curl_formadd( &formpost,
					  &lastptr,
					  CURLFORM_COPYNAME, "api_key",
					  CURLFORM_COPYCONTENTS, api_key,
					  CURLFORM_END );

		// Set Secret Key.
		curl_formadd( &formpost,
					  &lastptr,
					  CURLFORM_COPYNAME, "api_secret",
					  CURLFORM_COPYCONTENTS, api_secret,
					  CURLFORM_END );

		// Fill in the file upload field.
		curl_formadd( &formpost,
					  &lastptr,
					  CURLFORM_COPYNAME, "file",
					  CURLFORM_FILE, file,
					  CURLFORM_END );

		curl = curl_easy_init();

		if ( curl ) 
		{
			curl_easy_setopt( curl, CURLOPT_URL, "url" );
			curl_easy_setopt( curl, CURLOPT_HTTPPOST, formpost );
			curl_easy_setopt( curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_BASIC );
			curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback );
			curl_easy_setopt( curl, CURLOPT_WRITEDATA, (void *)&chunk);

			// The output from the example URL is easier to read as plain text.
			struct curl_slist *headers = NULL;
			headers = curl_slist_append( headers, "Accept: text/plain" );
			curl_easy_setopt( curl, CURLOPT_HTTPHEADER, headers );

			// Make the example URL work even if your CA bundle is missing.
			curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, 0L );

			res = curl_easy_perform( curl );

			if ( res != CURLE_OK ) 
			{
				GLog->Log( FString::Printf( TEXT( "curl_easy_perform() failed: %s" ), curl_easy_strerror(res) ) );
			}
			else 
			{
				char* msg = (char*)malloc( strlen( chunk.memory ) + 10 );
				sprintf( msg, "Response:\n%s", chunk.memory );
				GLog->Log( msg );
			}

			// Remember to call the appropriate "free" functions.
			curl_slist_free_all( headers );
			curl_easy_cleanup( curl );
			curl_formfree( formpost );
			free( chunk.memory );
			curl_global_cleanup();
		}
	}

	static void SubmitCapture( const char* api_key, const char* api_secret, 
								const char* uid, const char* name, const char* user_email, 
								const char* config, const char* image )
	{
		GLog->Log( "Submitting capture..." );

		CURL *curl;
		CURLcode res;

		// Memory to save the response.
		struct MemoryStruct chunk;
		chunk.memory = (char *)malloc(1);
		chunk.size = 0;
		chunk.memory[chunk.size] = 0;

		// HTTP form.
		struct curl_httppost *formpost = NULL;
		struct curl_httppost *lastptr = NULL;

		curl_global_init( CURL_GLOBAL_ALL );

		// Set AppID.
		curl_formadd( &formpost,
					  &lastptr,
					  CURLFORM_COPYNAME, "api_key",
					  CURLFORM_COPYCONTENTS, api_key,
					  CURLFORM_END );

		// Set Secret Key.
		curl_formadd( &formpost,
					  &lastptr,
					  CURLFORM_COPYNAME, "api_secret",
					  CURLFORM_COPYCONTENTS, api_secret,
					  CURLFORM_END );

		// Set Uid.
		curl_formadd( &formpost,
					  &lastptr,
					  CURLFORM_COPYNAME, "uid",
					  CURLFORM_COPYCONTENTS, uid,
					  CURLFORM_END );

		// Set name.
		curl_formadd( &formpost,
					  &lastptr,
					  CURLFORM_COPYNAME, "name",
					  CURLFORM_COPYCONTENTS, name,
					  CURLFORM_END );

		// Set user_email.
		curl_formadd( &formpost,
					  &lastptr,
					  CURLFORM_COPYNAME, "user_email",
					  CURLFORM_COPYCONTENTS, user_email,
					  CURLFORM_END );

		// Fill in the file upload field.
		curl_formadd( &formpost,
					  &lastptr,
					  CURLFORM_COPYNAME, "config",
					  CURLFORM_FILE, config,
					  CURLFORM_END );

		// Fill in the Item[image] upload field.
		curl_formadd( &formpost,
					  &lastptr,
					  CURLFORM_COPYNAME, "Item[image]",
					  CURLFORM_FILE, image,
					  CURLFORM_END );

		curl = curl_easy_init();

		if ( curl ) 
		{
			curl_easy_setopt( curl, CURLOPT_URL, "url" );
			curl_easy_setopt( curl, CURLOPT_HTTPPOST, formpost );
			curl_easy_setopt( curl, CURLOPT_HTTPAUTH, (long)CURLAUTH_BASIC );
			curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback );
			curl_easy_setopt( curl, CURLOPT_WRITEDATA, (void *)&chunk);

			// The output from the example URL is easier to read as plain text.
			struct curl_slist *headers = NULL;
			headers = curl_slist_append( headers, "Accept: text/plain" );
			curl_easy_setopt( curl, CURLOPT_HTTPHEADER, headers );

			// Make the example URL work even if your CA bundle is missing.
			curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, 0L );

			res = curl_easy_perform( curl );

			if ( res != CURLE_OK ) 
			{
				GLog->Log( FString::Printf( TEXT( "curl_easy_perform() failed: %s" ), curl_easy_strerror(res) ) );
			}
			else 
			{
				char* msg = (char*)malloc( strlen( chunk.memory ) + 10 );
				sprintf( msg, "Response:\n%s", chunk.memory );
				GLog->Log( msg );
			}

			// Remember to call the appropriate "free" functions.
			curl_slist_free_all( headers );
			curl_easy_cleanup( curl );
			curl_formfree( formpost );
			free( chunk.memory );
			curl_global_cleanup();
		}
	}
};