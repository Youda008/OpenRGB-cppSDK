//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: data structures containing device information
//======================================================================================================================

#include "OpenRGB/DeviceInfo.hpp"

#include "Essential.hpp"

#include "ProtocolCommon.hpp"
#include "BinaryStream.hpp"
using own::BinaryOutputStream;
using own::BinaryInputStream;
#include "MiscUtils.hpp"

#include <string>
using std::string;
#include <sstream>
using std::ostringstream;  // flags to string


namespace orgb {


//======================================================================================================================
//  enum strings

const char * enumString( DeviceType type )
{
	static const char * const deviceTypeStr [] =
	{
		"Motherboard",
		"DRAM",
		"GPU",
		"Cooler",
		"LedStrip",
		"Keyboard",
		"Mouse",
		"MouseMat",
		"Headset",
		"HeadsetStand",
		"Gamepad",
		"Unknown",
	};

	if (uint( type ) <= uint( DeviceType::Unknown ))
		return deviceTypeStr[ uint( type ) ];
	else
		return "<invalid>";
}

string modeFlagsToString( uint32_t flags )
{
	ostringstream oss;

	bool isFirst = true;
	auto addFlag = [ &isFirst, &oss ]( const char * flagStr )
	{
		if (isFirst) {
			isFirst = false;
		} else {
			oss << " | ";
		}
		oss << flagStr;
	};

	if (flags & ModeFlags::HasSpeed)
		addFlag( "HasSpeed" );
	if (flags & ModeFlags::HasDirectionLR)
		addFlag( "HasDirectionLR" );
	if (flags & ModeFlags::HasDirectionUD)
		addFlag( "HasDirectionUD" );
	if (flags & ModeFlags::HasDirectionHV)
		addFlag( "HasDirectionHV" );
	if (flags & ModeFlags::HasBrightness)
		addFlag( "HasBrightness" );
	if (flags & ModeFlags::HasPerLedColor)
		addFlag( "HasPerLedColor" );
	if (flags & ModeFlags::HasModeSpecificColor)
		addFlag( "HasModeSpecificColor" );
	if (flags & ModeFlags::HasRandomColor)
		addFlag( "HasRandomColor" );

	return oss.str();
}

const char * enumString( Direction dir )
{
	static const char * const deviceTypeStr [] =
	{
		"Left",
		"Right",
		"Up",
		"Down",
		"Horizontal",
		"Vertical",
	};

	if (uint( dir ) <= uint( Direction::Vertical ))
		return deviceTypeStr[ uint( dir ) ];
	else
		return "<invalid>";
}

const char * enumString( ColorMode mode )
{
	static const char * const colorModeStr [] =
	{
		"None",
		"PerLed",
		"ModeSpecific",
		"Random",
	};

	if (uint( mode ) <= uint( ColorMode::Random ) )
		return colorModeStr[ uint( mode ) ];
	else
		return "<invalid>";
}

const char * enumString( ZoneType type )
{
	static const char * const zoneTypeStr [] =
	{
		"Single",
		"Linear",
		"Matrix",
	};

	if (uint( type ) <= uint( ZoneType::Matrix ))
		return zoneTypeStr[ uint( type ) ];
	else
		return "<invalid>";
}


//======================================================================================================================
//  enum validation

static bool isValidDeviceType( DeviceType type )
{
	//using Int = std::underlying_type< DeviceType >::type;
	return int( type ) >= int( DeviceType::Motherboard ) && int( type ) <= int( DeviceType::Unknown );
}

static bool isValidDirection( Direction dir, uint32_t modeFlags )
{
	bool allowedDirections [ uint( Direction::Vertical ) + 1 ] = {0};
	bool hasAnyDirections = false;

	if (modeFlags & ModeFlags::HasDirectionLR)
	{
		hasAnyDirections = true;
		allowedDirections[ uint( Direction::Left ) ] = true;
		allowedDirections[ uint( Direction::Right ) ] = true;
	}
	if (modeFlags & ModeFlags::HasDirectionUD)
	{
		hasAnyDirections = true;
		allowedDirections[ uint( Direction::Up ) ] = true;
		allowedDirections[ uint( Direction::Down ) ] = true;
	}
	if (modeFlags & ModeFlags::HasDirectionHV)
	{
		hasAnyDirections = true;
		allowedDirections[ uint( Direction::Horizontal ) ] = true;
		allowedDirections[ uint( Direction::Vertical ) ] = true;
	}

	// in case no direction flag is active, direction will be uninitialized value, so it can be anything
	if (!hasAnyDirections)
	{
		return true;
	}

	return allowedDirections[ uint( dir ) ] == true;
}

static bool isValidColorMode( ColorMode mode )
{
	return int( mode ) >= int( ColorMode::None ) && int( mode ) <= int( ColorMode::Random );
}

static bool isValidZoneType( ZoneType type )
{
	return int( type ) >= int( ZoneType::Single ) && int( type ) <= int( ZoneType::Matrix );
}


//======================================================================================================================
//  repeated message sub-sections

size_t ModeDescription::calcSize() const
{
	size_t size = 0;

	size += sizeofORGBString( name );
	size += sizeof( value );
	size += sizeof( flags );
	size += sizeof( speed_min );
	size += sizeof( speed_max );
	size += sizeof( colors_min );
	size += sizeof( colors_max );
	size += sizeof( speed );
	size += sizeof( direction );
	size += sizeof( color_mode );
	size += sizeofORGBArray( colors );

	return size;
}

void ModeDescription::serialize( BinaryOutputStream & stream ) const
{
	writeORGBString( stream, name );
	stream << value;
	stream << flags;
	stream << speed_min;
	stream << speed_max;
	stream << colors_min;
	stream << colors_max;
	stream << speed;
	stream << direction;
	stream << color_mode;
	writeORGBArray( stream, colors );
}

bool ModeDescription::deserialize( BinaryInputStream & stream )
{
	readORGBString( stream, name );
	stream >> value;
	stream >> flags;
	stream >> speed_min;
	stream >> speed_max;
	stream >> colors_min;
	stream >> colors_max;
	stream >> speed;
	stream >> direction;
	stream >> color_mode;
	readORGBArray( stream, colors );

	if (!isValidDirection( direction, flags ))
		stream.setFailed();
	if (!isValidColorMode( color_mode ))
		stream.setFailed();

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

size_t ZoneDescription::calcSize() const
{
	size_t size = 0;

	size += sizeofORGBString( name );
	size += sizeof( type );
	size += sizeof( leds_min );
	size += sizeof( leds_max );
	size += sizeof( leds_count );
	size += sizeof( matrix_length );
	if (matrix_length > 0)
	{
		size += sizeof( matrix_height );
		size += sizeof( matrix_width );
		size += sizeofVector( matrix_values );
	}

	return size;
}

void ZoneDescription::serialize( BinaryOutputStream & stream ) const
{
	writeORGBString( stream, name );
	stream << type;
	stream << leds_min;
	stream << leds_max;
	stream << leds_count;
	stream << matrix_length;
	if (matrix_length > 0)
	{
		stream << matrix_height;
		stream << matrix_width;
		for (auto val : matrix_values)
		{
			stream << val;
		}
	}
}

bool ZoneDescription::deserialize( BinaryInputStream & stream)
{
	readORGBString( stream, name );
	stream >> type;
	stream >> leds_min;
	stream >> leds_max;
	stream >> leds_count;
	stream >> matrix_length;
	if (matrix_length > 0)
	{
		stream >> matrix_height;
		stream >> matrix_width;
		size_t matrixSize = matrix_height * matrix_width;
		matrix_values.resize( matrixSize );
		for (size_t i = 0; i < matrixSize; ++i)
		{
			stream >> matrix_values[i];
		}
	}

	if (!isValidZoneType( type ))
		stream.setFailed();

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

size_t LEDDescription::calcSize() const
{
	size_t size = 0;

	size += sizeofORGBString( name );
	size += sizeof( value );

	return size;
}

void LEDDescription::serialize( BinaryOutputStream & stream ) const
{
	writeORGBString( stream, name );
	stream << value;
}

bool LEDDescription::deserialize( BinaryInputStream & stream )
{
	readORGBString( stream, name );
	stream >> value;

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

size_t DeviceDescription::calcSize() const
{
	size_t size = 0;

	size += sizeof( device_type );
	size += sizeofORGBString( name );
	size += sizeofORGBString( vendor );
	size += sizeofORGBString( description );
	size += sizeofORGBString( version );
	size += sizeofORGBString( serial );
	size += sizeofORGBString( location );
	size += sizeof( uint16_t );
	size += sizeof( active_mode );
	size += sizeofVector( modes );

	return 0;
}

void DeviceDescription::serialize( BinaryOutputStream & stream ) const
{
	stream << device_type;
	writeORGBString( stream, name );
	writeORGBString( stream, vendor );
	writeORGBString( stream, description );
	writeORGBString( stream, version );
	writeORGBString( stream, serial );
	writeORGBString( stream, location );
	stream << uint16_t( modes.size() );  // the size is not directly before the array, so it must be written manually
	stream << active_mode;
	for (const ModeDescription & mode : modes)
	{
		mode.serialize( stream );
	}
	writeORGBArray( stream, zones );
	writeORGBArray( stream, leds );
	writeORGBArray( stream, colors );
}

bool DeviceDescription::deserialize( BinaryInputStream & stream )
{
	stream >> device_type;
	readORGBString( stream, name );
	readORGBString( stream, vendor );
	readORGBString( stream, description );
	readORGBString( stream, version );
	readORGBString( stream, serial );
	readORGBString( stream, location );
	uint16_t num_modes;
	stream >> num_modes;  // the size is not directly before the array, so it must be read manually
	stream >> active_mode;
	modes.resize( num_modes );
	for (size_t i = 0; i < num_modes; ++i)
	{
		modes[i].deserialize( stream );
	}
	readORGBArray( stream, zones );
	readORGBArray( stream, leds );
	readORGBArray( stream, colors );

	if (!isValidDeviceType( device_type ))
		stream.setFailed();

	return !stream.hasFailed();
}


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
	indent( indentLevel + 1 ); printf( "type = %s;\n", enumString( zone.type ) );
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

void Mode::toProtocolDescription( ModeDescription & desc ) const
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
	indent( indentLevel + 1 ); printf( "flags = %s;\n", modeFlagsToString( mode.flags ).c_str() );
	indent( indentLevel + 1 ); printf( "direction = %s;\n", enumString( mode.direction ) );
	indent( indentLevel + 1 ); printf( "minSpeed = %u;\n", mode.minSpeed );
	indent( indentLevel + 1 ); printf( "maxSpeed = %u;\n", mode.maxSpeed );
	indent( indentLevel + 1 ); printf( "speed = %u;\n", mode.speed );
	indent( indentLevel + 1 ); printf( "minColors = %u;\n", mode.minColors );
	indent( indentLevel + 1 ); printf( "maxColors = %u;\n", mode.maxColors );
	indent( indentLevel + 1 ); printf( "colorMode = %s;\n", enumString( mode.colorMode ) );
	indent( indentLevel + 1 ); printf( "colors = {\n" );
	for (Color color : mode.colors)
	{
		indent( indentLevel + 2 ); print( color ); puts(",");
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
	indent( indentLevel + 1 ); printf( "type = %s;\n", enumString( device.type ) );
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
		indent( indentLevel + 2 ); print( color ); puts(",");
	}
	indent( indentLevel + 1 ); printf( "};\n" );
	indent( indentLevel ); printf( "},\n" );
}


//======================================================================================================================


} // namespace orgb
