//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Created on:  1.11.2020
// Description: data structures containing device information
//======================================================================================================================

#include "OpenRGB/private/DeviceInfo.hpp"

#include "Common.hpp"

#include "Utils.hpp"


namespace orgb {


//======================================================================================================================
//  LED

LED::LED( const Device & parent, uint32_t id, LEDDescription && desc )
:
	parent( parent ),
	id( id ),
	name( move( desc.name ) ),
	value( desc.value )
{}

void print( const LED & led, unsigned int indentLevel )
{
	indent( indentLevel ); printf( "[%u] = { name = \"%s\"; value = %u },\n", led.id, led.name.c_str(), led.value );
}


//======================================================================================================================
//  Zone

Zone::Zone( const Device & parent, uint32_t id, ZoneDescription && desc )
:
	parent( parent ),
	id( id ),
	name( move( desc.name ) ),
	type( desc.type ),
	minLeds( desc.leds_min ),
	maxLeds( desc.leds_max ),
	numLeds( desc.leds_count ),
	matrixHeight( desc.matrix_height ),
	matrixWidth( desc.matrix_width )
	// TODO: matrix itself when it's clear what it contains
{}

void print( const Zone & zone, unsigned int indentLevel )
{
	indent( indentLevel ); printf( "[%u] = {\n", zone.id );
	indent( indentLevel + 1 ); printf( "name = \"%s\";\n", zone.name.c_str() );
	indent( indentLevel + 1 ); printf( "type = %u;\n", uint( zone.type ) );
	indent( indentLevel + 1 ); printf( "minLeds = %u;\n", zone.minLeds );
	indent( indentLevel + 1 ); printf( "maxLeds = %u;\n", zone.maxLeds );
	indent( indentLevel + 1 ); printf( "numLeds = %u;\n", zone.numLeds );
	indent( indentLevel + 1 ); printf( "matrixHeight = %u;\n", zone.matrixHeight );
	indent( indentLevel + 1 ); printf( "matrixWidth = %u;\n", zone.matrixWidth );
	indent( indentLevel ); printf( "},\n" );
}


//======================================================================================================================
//  Mode

Mode::Mode( const Device & parent, uint32_t id, ModeDescription && desc )
:
	parent( parent ),
	id( id ),
	name( move( desc.name ) ),
	value( desc.value ),
	flags( desc.flags ),
	direction( desc.direction ),
	minSpeed( desc.speed_min ),
	maxSpeed( desc.speed_max ),
	speed( desc.speed ),
	minColors( desc.colors_min ),
	maxColors( desc.colors_max ),
	colorMode( desc.color_mode ),
	colors( move( desc.colors ) )
{}

void Mode::toProtocolDescription( ModeDescription & desc )
{
	desc.name = name;
	desc.value = value;
	desc.flags = flags;
	desc.speed_min = minSpeed;
	desc.speed_max = maxSpeed;
	desc.colors_min = minColors;
	desc.colors_max = maxColors;
	desc.speed = speed;
	desc.direction = direction;
	desc.color_mode = colorMode;
	desc.colors = colors;
}

void print( const Mode & mode, unsigned int indentLevel )
{
	indent( indentLevel ); printf( "[%u] = {\n", mode.id );
	indent( indentLevel + 1 ); printf( "name = \"%s\";\n", mode.name.c_str() );
	indent( indentLevel + 1 ); printf( "value = %u;\n", mode.value );
	indent( indentLevel + 1 ); printf( "flags = %u;\n", mode.flags );
	indent( indentLevel + 1 ); printf( "direction = %u;\n", uint( mode.direction ) );
	indent( indentLevel + 1 ); printf( "minSpeed = %u;\n", mode.minSpeed );
	indent( indentLevel + 1 ); printf( "maxSpeed = %u;\n", mode.maxSpeed );
	indent( indentLevel + 1 ); printf( "speed = %u;\n", mode.speed );
	indent( indentLevel + 1 ); printf( "minColors = %u;\n", mode.minColors );
	indent( indentLevel + 1 ); printf( "maxColors = %u;\n", mode.maxColors );
	indent( indentLevel + 1 ); printf( "colorMode = %u;\n", uint( mode.colorMode ) );
	indent( indentLevel + 1 ); printf( "colors = {\n" );
	for (Color color : mode.colors)
	{
		print( color, indentLevel + 2 );
	}
	indent( indentLevel + 1 ); printf( "};\n" );
	indent( indentLevel ); printf( "},\n" );
}


//======================================================================================================================
//  Device

Device::Device( uint32_t id, DeviceDescription && desc )
:
	id( id ),
	type( desc.device_type ),
	name( move( desc.name ) ),
	description( move( desc.description ) ),
	version( move( desc.version ) ),
	serial( move( desc.serial ) ),
	location( move( desc.location ) ),
	activeMode( desc.active_mode )
{
	// create modes
	for (uint32_t modeIdx = 0; modeIdx < desc.modes.size(); ++modeIdx)
	{
		modes.emplace_back( *this, modeIdx, move( desc.modes[ modeIdx ] ) );
	}

	// create zones
	for (uint32_t zoneIdx = 0; zoneIdx < desc.zones.size(); ++zoneIdx)
	{
		zones.emplace_back( *this, zoneIdx, move( desc.zones[ zoneIdx ] ) );
	}

	// create leds
	for (uint32_t ledIdx = 0; ledIdx < desc.leds.size(); ++ledIdx)
	{
		leds.emplace_back( *this, ledIdx, move( desc.leds[ ledIdx ] ) );
	}
}

void print( const Device & device, unsigned int indentLevel )
{
	indent( indentLevel ); printf( "[%u] = {\n", device.id );
	indent( indentLevel + 1 ); printf( "name = \"%s\";\n", device.name.c_str() );
	indent( indentLevel + 1 ); printf( "type = %u;\n", uint( device.type ) );
	indent( indentLevel + 1 ); printf( "description = \"%s\";\n", device.description.c_str() );
	indent( indentLevel + 1 ); printf( "version = \"%s\";\n", device.version.c_str() );
	indent( indentLevel + 1 ); printf( "serial = \"%s\";\n", device.serial.c_str() );
	indent( indentLevel + 1 ); printf( "location = \"%s\";\n", device.location.c_str() );
	indent( indentLevel + 1 ); printf( "activeMode = %u;\n", device.activeMode );
	indent( indentLevel + 1 ); printf( "modes = {\n" );
	for (const Mode & mode : device.modes)
	{
		print( mode, indentLevel + 2 );
	}
	indent( indentLevel + 1 ); printf( "};\n" );
	indent( indentLevel + 1 ); printf( "zones = {\n" );
	for (const Zone & zone : device.zones)
	{
		print( zone, indentLevel + 2 );
	}
	indent( indentLevel + 1 ); printf( "};\n" );
	indent( indentLevel + 1 ); printf( "leds = {\n" );
	for (const LED & led : device.leds)
	{
		print( led, indentLevel + 2 );
	}
	indent( indentLevel + 1 ); printf( "};\n" );
	indent( indentLevel + 1 ); printf( "colors = {\n" );
	for (Color color : device.colors)
	{
		print( color, indentLevel + 2 );
	}
	indent( indentLevel + 1 ); printf( "};\n" );
	indent( indentLevel ); printf( "},\n" );
}


//======================================================================================================================


} // namespace orgb
