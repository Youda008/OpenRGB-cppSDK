//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Created on:  3.11.2020
// Description: Representation of a color
//======================================================================================================================

#include "OpenRGB/private/Color.hpp"

#include "Common.hpp"

#include "BufferStream.hpp"
#include "Utils.hpp"


namespace orgb {


//======================================================================================================================
//  Color

const Color Color::BLACK   (0x00, 0x00, 0x00);
const Color Color::WHITE   (0xFF, 0xFF, 0xFF);
const Color Color::RED     (0xFF, 0x00, 0x00);
const Color Color::GREEN   (0x00, 0xFF, 0x00);
const Color Color::BLUE    (0x00, 0x00, 0xFF);
const Color Color::YELLOW  (0xFF, 0xFF, 0x00);
const Color Color::MAGENTA (0xFF, 0x00, 0xFF);
const Color Color::CYAN    (0x00, 0xFF, 0xFF);

void Color::serialize( BufferOutputStream & stream ) const
{
	uint8_t padding = 0;
	stream << r << g << b << padding;
}

bool Color::deserialize( BufferInputStream & stream )
{
	uint8_t padding;
	stream >> r >> g >> b >> padding;

	return !stream.hasFailed();
}

void print( const Color & color, unsigned int indentLevel )
{
	printf( "%02X%02X%02X", color.r, color.g, color.b );
}


//======================================================================================================================


} // namespace orgb
