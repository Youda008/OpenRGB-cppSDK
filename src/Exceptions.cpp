//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: exceptions thrown by OpenRGB network client
//======================================================================================================================

#ifndef NO_EXCEPTIONS

#include "OpenRGB/Exceptions.hpp"

#include "Essential.hpp"

#include "SystemErrorInfo.hpp"
using own::getLastError;
using own::getErrorString;


namespace orgb {


//======================================================================================================================

const char * Exception::errorMessage() const noexcept
{
	return _message;
}


//======================================================================================================================


} // namespace orgb


#endif // NO_EXCEPTIONS
