//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: miscellaneous utilities
//======================================================================================================================

#include "MiscUtils.hpp"

#include <cstdio>


namespace orgb {


void indent( unsigned int indentLevel )
{
	for (uint i = 0; i < indentLevel; ++i)
		putchar('\t');
}


} // namespace orgb
