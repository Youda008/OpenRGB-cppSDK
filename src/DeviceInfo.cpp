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

const char * enumString( DeviceType type ) noexcept
{
	static const char * const DeviceTypeStr [] =
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
	static_assert( size_t(DeviceType::Unknown) + 1 == own::size(DeviceTypeStr), "update the DeviceTypeStr" );

	// Collapse everything else than known classes to Unknown
	// in case the server adds some classes without increasing protocol version.
	if (size_t( type ) < size_t( DeviceType::Unknown ))
		return DeviceTypeStr[ size_t( type ) ];
	else
		return DeviceTypeStr[ size_t( DeviceType::Unknown ) ];
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

const char * enumString( Direction dir ) noexcept
{
	static const char * const DirectionStr [] =
	{
		"Left",
		"Right",
		"Up",
		"Down",
		"Horizontal",
		"Vertical",
	};
	static_assert( size_t(Direction::Vertical) + 1 == own::size(DirectionStr), "update the DirectionStr" );

	if (size_t( dir ) <= size_t( Direction::Vertical ))
		return DirectionStr[ size_t( dir ) ];
	else
		return "<invalid>";
}

const char * enumString( ColorMode mode ) noexcept
{
	static const char * const ColorModeStr [] =
	{
		"None",
		"PerLed",
		"ModeSpecific",
		"Random",
	};
	static_assert( size_t(ColorMode::Random) + 1 == own::size(ColorModeStr), "update the ColorModeStr" );

	if (size_t( mode ) <= size_t( ColorMode::Random ) )
		return ColorModeStr[ size_t( mode ) ];
	else
		return "<invalid>";
}

const char * enumString( ZoneType type ) noexcept
{
	static const char * const ZoneTypeStr [] =
	{
		"Single",
		"Linear",
		"Matrix",
	};
	static_assert( size_t(ZoneType::Matrix) + 1 == own::size(ZoneTypeStr), "update the ZoneTypeStr" );

	if (size_t( type ) <= size_t( ZoneType::Matrix ))
		return ZoneTypeStr[ size_t( type ) ];
	else
		return "<invalid>";
}


//======================================================================================================================
//  enum validation

/*static bool isValidDeviceType( DeviceType type )
{
	return size_t( type ) <= size_t( DeviceType::Unknown );
}*/

static bool isValidDirection( Direction dir, uint32_t modeFlags )
{
	bool allowedDirections [ size_t( Direction::Vertical ) + 1 ] = {0};
	bool hasAnyDirections = false;

	if (modeFlags & ModeFlags::HasDirectionLR)
	{
		hasAnyDirections = true;
		allowedDirections[ size_t( Direction::Left ) ] = true;
		allowedDirections[ size_t( Direction::Right ) ] = true;
	}
	if (modeFlags & ModeFlags::HasDirectionUD)
	{
		hasAnyDirections = true;
		allowedDirections[ size_t( Direction::Up ) ] = true;
		allowedDirections[ size_t( Direction::Down ) ] = true;
	}
	if (modeFlags & ModeFlags::HasDirectionHV)
	{
		hasAnyDirections = true;
		allowedDirections[ size_t( Direction::Horizontal ) ] = true;
		allowedDirections[ size_t( Direction::Vertical ) ] = true;
	}

	// in case no direction flag is active, direction will be uninitialized value, so it can be anything
	if (!hasAnyDirections)
	{
		return true;
	}

	return allowedDirections[ size_t( dir ) ] == true;
}

static bool isValidColorMode( ColorMode mode )
{
	return size_t( mode ) <= size_t( ColorMode::Random );
}

static bool isValidZoneType( ZoneType type )
{
	return size_t( type ) <= size_t( ZoneType::Matrix );
}


//======================================================================================================================
//  repeated message sub-sections

size_t ModeDescription::calcSize() const noexcept
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

bool ModeDescription::deserialize( BinaryInputStream & stream ) noexcept
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

size_t ZoneDescription::calcSize() const noexcept
{
	size_t size = 0;

	size += sizeofORGBString( name );
	size += sizeof( type );
	size += sizeof( leds_min );
	size += sizeof( leds_max );
	size += sizeof( leds_count );

	size += sizeof( uint16_t );  // length of the optional matrix block
	if (matrix_values.size() > 0)
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

	uint16_t matrix_length = uint16_t(
		matrix_values.size() > 0 ? sizeof( matrix_height ) + sizeof( matrix_width ) + sizeofVector( matrix_values ) : 0
	);
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

bool ZoneDescription::deserialize( BinaryInputStream & stream) noexcept
{
	readORGBString( stream, name );
	stream >> type;
	stream >> leds_min;
	stream >> leds_max;
	stream >> leds_count;

	uint16_t matrix_length;
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

size_t LEDDescription::calcSize() const noexcept
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

bool LEDDescription::deserialize( BinaryInputStream & stream ) noexcept
{
	readORGBString( stream, name );
	stream >> value;

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

size_t DeviceDescription::calcSize() const noexcept
{
	size_t size = 0;

	size += sizeof( type );
	size += sizeofORGBString( name );
	size += sizeofORGBString( vendor );
	size += sizeofORGBString( description );
	size += sizeofORGBString( version );
	size += sizeofORGBString( serial );
	size += sizeofORGBString( location );
	size += sizeof( uint16_t );
	size += sizeof( active_mode );
	size += sizeofVector( modes );

	return size;
}

void DeviceDescription::serialize( BinaryOutputStream & stream ) const
{
	stream << type;
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

bool DeviceDescription::deserialize( BinaryInputStream & stream ) noexcept
{
	stream >> type;
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

	// Let's tolerate invalid device classes in case the server adds some without increasing protocol version
	//if (!isValidDeviceType( type ))
	//	stream.setFailed();

	return !stream.hasFailed();
}


//======================================================================================================================
//  LED

LED::LED( const Device & parent, uint32_t idx, LEDDescription && descr ) noexcept
:
	parent( parent ),
	idx( idx ),
	desc( move(descr) )
{}

void print( const LED & led, unsigned int indentLevel )
{
	indent( indentLevel ); printf( "[%u] = { name = \"%s\"; value = %u },\n", led.idx, led.desc.name.c_str(), led.desc.value );
}

void print( std::ostream & os, const LED & led, unsigned int indentLevel )
{
	indent( os, indentLevel ); os << "["<<led.idx<<"] = { name = \""<<led.desc.name<<"\"; value = "<<led.desc.value<<" },\n";
}


//======================================================================================================================
//  Zone

Zone::Zone( const Device & parent, uint32_t idx, ZoneDescription && descr ) noexcept
:
	parent( parent ),
	idx( idx ),
	desc( move(descr) )
{}

void print( const Zone & zone, unsigned int indentLevel )
{
	indent( indentLevel ); printf( "[%u] = {\n", zone.idx );
	indent( indentLevel + 1 ); printf( "name = \"%s\";\n", zone.desc.name.c_str() );
	indent( indentLevel + 1 ); printf( "type = %s;\n", enumString( zone.desc.type ) );
	indent( indentLevel + 1 ); printf( "leds_min = %u;\n", zone.desc.leds_min );
	indent( indentLevel + 1 ); printf( "leds_max = %u;\n", zone.desc.leds_max );
	indent( indentLevel + 1 ); printf( "leds_count = %u;\n", zone.desc.leds_count );
	indent( indentLevel + 1 ); printf( "matrix_height = %u;\n", zone.desc.matrix_height );
	indent( indentLevel + 1 ); printf( "matrix_width = %u;\n", zone.desc.matrix_width );
	indent( indentLevel ); printf( "},\n" );
}

void print( std::ostream & os, const Zone & zone, unsigned int indentLevel )
{
	indent( os, indentLevel ); os << "["<<zone.idx<<"] = {\n";
	indent( os, indentLevel + 1 ); os << "name = \""<<zone.desc.name.c_str()<<"\";\n";
	indent( os, indentLevel + 1 ); os << "type = "<<enumString( zone.desc.type )<<";\n";
	indent( os, indentLevel + 1 ); os << "leds_min = "<<zone.desc.leds_min<<";\n";
	indent( os, indentLevel + 1 ); os << "leds_max = "<<zone.desc.leds_max<<";\n";
	indent( os, indentLevel + 1 ); os << "leds_count = "<<zone.desc.leds_count<<";\n";
	indent( os, indentLevel + 1 ); os << "matrix_height = "<<zone.desc.matrix_height<<";\n";
	indent( os, indentLevel + 1 ); os << "matrix_width = "<<zone.desc.matrix_width<<";\n";
	indent( os, indentLevel ); os << "},\n";
}


//======================================================================================================================
//  Mode

Mode::Mode( const Device & parent, uint32_t idx, ModeDescription && descr ) noexcept
:
	parent( parent ),
	idx( idx ),
	desc( move(descr) ),
	direction( desc.direction ),
	speed( desc.speed ),
	colors( desc.colors )
{}

void Mode::toProtocolDescription( ModeDescription & descr ) const
{
	descr = desc;
	descr.direction = direction;
	descr.speed = speed;
	descr.colors = colors;
}

void print( const Mode & mode, unsigned int indentLevel )
{
	indent( indentLevel ); printf( "[%u] = {\n", mode.idx );
	indent( indentLevel + 1 ); printf( "name = \"%s\";\n", mode.desc.name.c_str() );
	indent( indentLevel + 1 ); printf( "value = %u;\n", mode.desc.value );
	indent( indentLevel + 1 ); printf( "flags = %s;\n", modeFlagsToString( mode.desc.flags ).c_str() );
	indent( indentLevel + 1 ); printf( "direction = %s;\n", enumString( mode.direction ) );
	indent( indentLevel + 1 ); printf( "speed_min = %u;\n", mode.desc.speed_min );
	indent( indentLevel + 1 ); printf( "speed_max = %u;\n", mode.desc.speed_max );
	indent( indentLevel + 1 ); printf( "speed = %u;\n", mode.desc.speed );
	indent( indentLevel + 1 ); printf( "colors_min = %u;\n", mode.desc.colors_min );
	indent( indentLevel + 1 ); printf( "colors_max = %u;\n", mode.desc.colors_max );
	indent( indentLevel + 1 ); printf( "color_mode = %s;\n", enumString( mode.desc.color_mode ) );
	indent( indentLevel + 1 ); printf( "colors = {\n" );
	for (Color color : mode.colors)
	{
		indent( indentLevel + 2 ); print( color ); puts(",");
	}
	indent( indentLevel + 1 ); printf( "};\n" );
	indent( indentLevel ); printf( "},\n" );
}

void print( std::ostream & os, const Mode & mode, unsigned int indentLevel )
{
	indent( os, indentLevel ); os << "["<<mode.idx<<"] = {\n";
	indent( os, indentLevel + 1 ); os << "name = \""<<mode.desc.name<<"\";\n";
	indent( os, indentLevel + 1 ); os << "value = "<<mode.desc.value<<";\n";
	indent( os, indentLevel + 1 ); os << "flags = "<<modeFlagsToString( mode.desc.flags )<<";\n";
	indent( os, indentLevel + 1 ); os << "direction = "<<enumString( mode.direction )<<";\n";
	indent( os, indentLevel + 1 ); os << "speed_min = "<<mode.desc.speed_min<<";\n";
	indent( os, indentLevel + 1 ); os << "speed_max = "<<mode.desc.speed_max<<";\n";
	indent( os, indentLevel + 1 ); os << "speed = "<<mode.speed<<";\n";
	indent( os, indentLevel + 1 ); os << "colors_min = "<<mode.desc.colors_min<<";\n";
	indent( os, indentLevel + 1 ); os << "colors_max = "<<mode.desc.colors_max<<";\n";
	indent( os, indentLevel + 1 ); os << "color_mode = "<<enumString( mode.desc.color_mode )<<";\n";
	indent( os, indentLevel + 1 ); os << "colors = {\n";
	for (Color color : mode.colors)
	{
		indent( os, indentLevel + 2 ); os << color << ",\n";
	}
	indent( os, indentLevel + 1 ); os << "};\n";
	indent( os, indentLevel ); os << "},\n";
}


//======================================================================================================================
//  Device

Device::Device( uint32_t idx, DeviceDescription && descr )
:
	idx( idx ),
	desc( move(descr) )
{
	// create modes
	for (uint32_t modeIdx = 0; modeIdx < desc.modes.size(); ++modeIdx)
	{
		modes.emplace_back( *this, modeIdx, move( desc.modes[ modeIdx ] ) );
	}
	desc.modes.clear();

	// create zones
	for (uint32_t zoneIdx = 0; zoneIdx < desc.zones.size(); ++zoneIdx)
	{
		zones.emplace_back( *this, zoneIdx, move( desc.zones[ zoneIdx ] ) );
	}
	desc.zones.clear();

	// create leds
	for (uint32_t ledIdx = 0; ledIdx < desc.leds.size(); ++ledIdx)
	{
		leds.emplace_back( *this, ledIdx, move( desc.leds[ ledIdx ] ) );
	}
	desc.leds.clear();
}

void print( const Device & device, unsigned int indentLevel )
{
	indent( indentLevel ); printf( "[%u] = {\n", device.idx );
	indent( indentLevel + 1 ); printf( "name = \"%s\";\n", device.desc.name.c_str() );
	indent( indentLevel + 1 ); printf( "type = %s;\n", enumString( device.desc.type ) );
	indent( indentLevel + 1 ); printf( "vendor = \"%s\";\n", device.desc.vendor.c_str() );
	indent( indentLevel + 1 ); printf( "description = \"%s\";\n", device.desc.description.c_str() );
	indent( indentLevel + 1 ); printf( "version = \"%s\";\n", device.desc.version.c_str() );
	indent( indentLevel + 1 ); printf( "serial = \"%s\";\n", device.desc.serial.c_str() );
	indent( indentLevel + 1 ); printf( "location = \"%s\";\n", device.desc.location.c_str() );
	indent( indentLevel + 1 ); printf( "active_mode = %u;\n", device.desc.active_mode );
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

void print( std::ostream & os, const Device & device, unsigned int indentLevel )
{
	indent( os, indentLevel ); os << "["<<device.idx<<"] = {\n";
	indent( os, indentLevel + 1 ); os << "name = \""<<device.desc.name<<"\";\n";
	indent( os, indentLevel + 1 ); os << "type = "<<enumString( device.desc.type )<<";\n";
	indent( os, indentLevel + 1 ); os << "vendor = \""<<device.desc.vendor<<"\";\n";
	indent( os, indentLevel + 1 ); os << "description = \""<<device.desc.description<<"\";\n";
	indent( os, indentLevel + 1 ); os << "version = \""<<device.desc.version<<"\";\n";
	indent( os, indentLevel + 1 ); os << "serial = \""<<device.desc.serial<<"\";\n";
	indent( os, indentLevel + 1 ); os << "location = \""<<device.desc.location<<"\";\n";
	indent( os, indentLevel + 1 ); os << "active_mode = "<<device.desc.active_mode<<";\n";
	indent( os, indentLevel + 1 ); os << "modes = {\n";
	for (const Mode & mode : device.modes)
	{
		print( os, mode, indentLevel + 2 );
	}
	indent( os, indentLevel + 1 ); os << "};\n";
	indent( os, indentLevel + 1 ); os << "zones = {\n";
	for (const Zone & zone : device.zones)
	{
		print( os, zone, indentLevel + 2 );
	}
	indent( os, indentLevel + 1 ); os << "};\n";
	indent( os, indentLevel + 1 ); os << "leds = {\n";
	for (const LED & led : device.leds)
	{
		print( os, led, indentLevel + 2 );
	}
	indent( os, indentLevel + 1 ); os << "};\n";
	indent( os, indentLevel + 1 ); os << "colors = {\n";
	for (Color color : device.colors)
	{
		indent( os, indentLevel + 2 ); os << color << ",\n";
	}
	indent( os, indentLevel + 1 ); os << "};\n";
	indent( os, indentLevel ); os << "},\n";
}


//======================================================================================================================


} // namespace orgb
