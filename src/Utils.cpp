//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Created on:  1.11.2020
// Description: miscellaneous utilities
//======================================================================================================================

#include "Utils.hpp"

#include <cstdio>


namespace orgb {


void indent( unsigned int indentLevel )
{
	for (uint i = 0; i < indentLevel; ++i)
		putchar('\t');
}


} // namespace orgb
