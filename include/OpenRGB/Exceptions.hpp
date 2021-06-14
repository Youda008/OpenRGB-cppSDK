//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: exceptions thrown by OpenRGB network client
//======================================================================================================================

#ifndef OPENRGB_EXCEPTIONS_INCLUDED
#define OPENRGB_EXCEPTIONS_INCLUDED


#include "SystemErrorType.hpp"  // HACK: read the comment at the top of that header file


namespace orgb {


//======================================================================================================================

/** Base class for all OpenRGB Client exceptions */
class Exception
{
 public:
	Exception( const char * message ) : _message( message ) {}
	virtual ~Exception() = default;
	virtual const char * errorMessage() const;
 protected:
	const char * _message;
};


/** Invalid usage of the client. */
class UserError : public Exception
{
 public:
	UserError( const char * message ) : Exception( message ) {}
};


/** Network error that prevented the client to connect or perform a request. */
class ConnectionError : public Exception
{
 public:
	ConnectionError( const char * message, system_error_t systemErrorCode )
		: Exception( message ), _errorCode( systemErrorCode ) {}
	system_error_t systemErrorCode() const { return _errorCode; }
 protected:
	system_error_t _errorCode;
};


/** Error that occured inside the operating system. Check the error code for more info. */
class SystemError : public Exception
{
 public:
	SystemError( const char * message, system_error_t systemErrorCode )
		: Exception( message ), _errorCode( systemErrorCode ) {}
	system_error_t systemErrorCode() const { return _errorCode; }
 protected:
	system_error_t _errorCode;
};


/** This exception is thrown when the device, mode, zone or LED you searched for was not found. */
class NotFound : public Exception
{
 public:
	NotFound( const char * message ) : Exception( message ) {}
};


//======================================================================================================================


} // namespace orgb


#endif // OPENRGB_EXCEPTIONS_INCLUDED
