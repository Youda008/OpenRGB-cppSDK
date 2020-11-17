//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Created on:  3.11.2020
// Description: Representation of a color
//======================================================================================================================

#include "OpenRGB/Color.hpp"

#include "Essential.hpp"

#include "MiscUtils.hpp"
#include "BufferStream.hpp"
using own::BufferOutputStream;
using own::BufferInputStream;


namespace orgb {


//======================================================================================================================
//  Color

const Color Color::Black   (0x00, 0x00, 0x00);
const Color Color::White   (0xFF, 0xFF, 0xFF);
const Color Color::Red     (0xFF, 0x00, 0x00);
const Color Color::Green   (0x00, 0xFF, 0x00);
const Color Color::Blue    (0x00, 0x00, 0xFF);
const Color Color::Yellow  (0xFF, 0xFF, 0x00);
const Color Color::Magenta (0xFF, 0x00, 0xFF);
const Color Color::Cyan    (0x00, 0xFF, 0xFF);

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

void print( const Color & color, unsigned int /*indentLevel*/ )
{
	printf( "%02X%02X%02X", color.r, color.g, color.b );
}


//======================================================================================================================


} // namespace orgb
