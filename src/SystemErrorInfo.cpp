//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Created on:  1.11.2020
// Description: universal way of getting information about system errors
//======================================================================================================================

#include "OpenRGB/private/SystemErrorInfo.hpp"

#ifdef _WIN32
	#include <windows.h>
#else
	#include <cerrno>
	#include <cstring>
#endif // _WIN32


namespace orgb {


//======================================================================================================================

system_error_t getLastError()
{
 #ifdef _WIN32
	return GetLastError();
 #else
	return errno;
 #endif // _WIN32
}

std::string getErrorString( system_error_t errorCode )
{
	char outStr [256];
 #ifdef _WIN32
	DWORD length;
	length = FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM,                     // It´s a system error
	                         nullptr,                                        // No string to be formatted needed
	                         DWORD( errorCode ),                             // Hey Windows: Please explain this error!
	                         MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ), // Do it in the english language
	                         outStr,                                         // Put the message here
	                         sizeof(outStr) - 1,                             // Number of bytes to store the message
	                         nullptr
						  );
	outStr[ length - 2 ] = '\0'; // cut CR LF
 #else
	strncpy( outStr, strerror(errorCode), sizeof(outStr) );
	outStr[ sizeof(outStr) - 1 ] = '\0';
 #endif // _WIN32
	return std::string( outStr );
}


//======================================================================================================================


} // namespace orgb
