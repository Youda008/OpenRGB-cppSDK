//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Created on:  1.11.2020
// Description: miscellaneous utilities
//======================================================================================================================

#include "OpenRGB/private/Protocol.hpp"

#include "BufferStream.hpp"
#include "Utils.hpp"

#include <cstring>  // strncmp
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <sstream>
using std::ostringstream;


namespace orgb {


//======================================================================================================================
//  OpenRGB strings are made of 2 bytes of little-endian length and the string itself, terminated by '\0' character

static size_t sizeofORGBString( const string & str )
{
	return 2 + str.size() + 1;
}

static void writeORGBString( BufferOutputStream & stream, const string & str )
{
	stream.reserveAdditional( 2 + str.size() + 1 );
	stream.writeIntLE( uint16_t(str.size()) );
	stream.writeString0( str );
}

static bool readORGBString( BufferInputStream & stream, string & str )
{
	uint16_t size = 0;
	stream.readIntLE( size );
	stream.readString0( str );
	return !stream.hasFailed() && str.size() == size;
}


//======================================================================================================================
//  serialize/deserialize arrays

template< typename Type, typename std::enable_if< std::is_fundamental<Type>::value, int >::type = 0 >
static size_t sizeofVector( const vector< Type > & vec )
{
	return vec.size() * sizeof( Type );
}

template< typename Type, typename std::enable_if< !std::is_fundamental<Type>::value, int >::type = 0 >
static size_t sizeofVector( const vector< Type > & vec )
{
	size_t size = 0;
	for (const Type & elem : vec)
	{
		size += elem.calcSize();
	}
	return size;
}

template< typename Type >
static size_t sizeofORGBArray( const vector< Type > & vec )
{
	return 2 + sizeofVector( vec );
}

template< typename Type, typename std::enable_if< std::is_integral<Type>::value, int >::type = 0 >
static void writeORGBArray( BufferOutputStream & stream, const vector< Type > vec )
{
	stream.reserveAdditional( 2 + vec.size() * sizeof(Type) );
	stream.writeIntLE( uint16_t(vec.size()) );
	for (const Type & elem : vec)
	{
		stream.writeIntLE( elem );
	}
}

template< typename Type, typename std::enable_if< !std::is_integral<Type>::value, int >::type = 0 >
static void writeORGBArray( BufferOutputStream & stream, const vector< Type > vec )
{
	stream.reserveAdditional( 2 + vec.size() * sizeof(Type) );
	stream.writeIntLE( uint16_t(vec.size()) );
	for (const Type & elem : vec)
	{
		elem.serialize( stream );
	}
}

template< typename Type, typename std::enable_if< std::is_integral<Type>::value, int >::type = 0 >
static bool readORGBArray( BufferInputStream & stream, vector< Type > & vec )
{
	uint16_t size = 0;
	stream.readIntLE( size );
	vec.resize( size );
	for (uint16_t i = 0; i < size; ++i)
	{
		stream.readIntLE( vec[i] );
	}
	return !stream.hasFailed();
}

template< typename Type, typename std::enable_if< !std::is_integral<Type>::value, int >::type = 0 >
static bool readORGBArray( BufferInputStream & stream, vector< Type > & vec )
{
	uint16_t size = 0;
	stream.readIntLE( size );
	vec.resize( size );
	for (uint16_t i = 0; i < size; ++i)
	{
		vec[i].deserialize( stream );
	}
	return !stream.hasFailed();
}


//======================================================================================================================
//  enum strings

const char * toString( MessageType type )
{
	// the values of message types are wildly different so we can't use an array
	switch (type)
	{
		case MessageType::REQUEST_CONTROLLER_COUNT:       return "REQUEST_CONTROLLER_COUNT";
		case MessageType::REQUEST_CONTROLLER_DATA:        return "REQUEST_CONTROLLER_DATA";
		case MessageType::SET_CLIENT_NAME:                return "SET_CLIENT_NAME";
		case MessageType::DEVICE_LIST_UPDATED:            return "DEVICE_LIST_UPDATED";
		case MessageType::RGBCONTROLLER_RESIZEZONE:       return "RGBCONTROLLER_RESIZEZONE";
		case MessageType::RGBCONTROLLER_UPDATELEDS:       return "RGBCONTROLLER_UPDATELEDS";
		case MessageType::RGBCONTROLLER_UPDATEZONELEDS:   return "RGBCONTROLLER_UPDATEZONELEDS";
		case MessageType::RGBCONTROLLER_UPDATESINGLELED:  return "RGBCONTROLLER_UPDATESINGLELED";
		case MessageType::RGBCONTROLLER_SETCUSTOMMODE:    return "RGBCONTROLLER_SETCUSTOMMODE";
		case MessageType::RGBCONTROLLER_UPDATEMODE:       return "RGBCONTROLLER_UPDATEMODE";
		default:                                          return "<invalid>";
	}
}

const char * toString( DeviceType type )
{
	static const char * const deviceTypeStr [] =
	{
		"MOTHERBOARD",
		"DRAM",
		"GPU",
		"COOLER",
		"LEDSTRIP",
		"KEYBOARD",
		"MOUSE",
		"MOUSEMAT",
		"HEADSET",
		"HEADSET_STAND",
		"GAMEPAD",
		"UNKNOWN",
	};

	if (uint( type ) <= uint( DeviceType::UNKNOWN ))
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

	if (flags & ModeFlags::HAS_SPEED)
		addFlag( "HAS_SPEED" );
	if (flags & ModeFlags::HAS_DIRECTION_LR)
		addFlag( "HAS_DIRECTION_LR" );
	if (flags & ModeFlags::HAS_DIRECTION_UD)
		addFlag( "HAS_DIRECTION_UD" );
	if (flags & ModeFlags::HAS_DIRECTION_HV)
		addFlag( "HAS_DIRECTION_HV" );
	if (flags & ModeFlags::HAS_BRIGHTNESS)
		addFlag( "HAS_BRIGHTNESS" );
	if (flags & ModeFlags::HAS_PER_LED_COLOR)
		addFlag( "HAS_PER_LED_COLOR" );
	if (flags & ModeFlags::HAS_MODE_SPECIFIC_COLOR)
		addFlag( "HAS_MODE_SPECIFIC_COLOR" );
	if (flags & ModeFlags::HAS_RANDOM_COLOR)
		addFlag( "HAS_RANDOM_COLOR" );

	return oss.str();
}

const char * toString( Direction dir )
{
	static const char * const deviceTypeStr [] =
	{
		"LEFT",
		"RIGHT",
		"UP",
		"DOWN",
		"HORIZONTAL",
		"VERTICAL",
	};

	if (uint( dir ) <= uint( Direction::VERTICAL ))
		return deviceTypeStr[ uint( dir ) ];
	else
		return "<invalid>";
}

const char * toString( ColorMode mode )
{
	static const char * const colorModeStr [] =
	{
		"NONE",
		"PER_LED",
		"MODE_SPECIFIC",
		"RANDOM",
	};

	if (uint( mode ) <= uint( ColorMode::RANDOM ) )
		return colorModeStr[ uint( mode ) ];
	else
		return "<invalid>";
}

const char * toString( ZoneType type )
{
	static const char * const zoneTypeStr [] =
	{
		"SINGLE",
		"LINEAR",
		"MATRIX",
	};

	if (uint( type ) <= uint( ZoneType::MATRIX ))
		return zoneTypeStr[ uint( type ) ];
	else
		return "<invalid>";
}


//======================================================================================================================
//  enum validation

static bool isValidMessageType( MessageType type )
{
	switch (type)
	{
		case MessageType::REQUEST_CONTROLLER_COUNT:      return true;
		case MessageType::REQUEST_CONTROLLER_DATA:       return true;
		case MessageType::SET_CLIENT_NAME:               return true;
		case MessageType::DEVICE_LIST_UPDATED:           return true;
		case MessageType::RGBCONTROLLER_RESIZEZONE:      return true;
		case MessageType::RGBCONTROLLER_UPDATELEDS:      return true;
		case MessageType::RGBCONTROLLER_UPDATEZONELEDS:  return true;
		case MessageType::RGBCONTROLLER_UPDATESINGLELED: return true;
		case MessageType::RGBCONTROLLER_SETCUSTOMMODE:   return true;
		case MessageType::RGBCONTROLLER_UPDATEMODE:      return true;
		default:                                         return false;
	}
}

static bool isValidDeviceType( DeviceType type )
{
	//using Int = std::underlying_type< DeviceType >::type;
	return int( type ) >= int( DeviceType::MOTHERBOARD ) && int( type ) <= int( DeviceType::UNKNOWN );
}

static bool isValidDirection( Direction dir, uint32_t modeFlags )
{
	bool allowedDirections [ uint( Direction::VERTICAL ) + 1 ] = {0};
	bool hasAnyDirections = false;

	if (modeFlags & ModeFlags::HAS_DIRECTION_LR)
	{
		hasAnyDirections = true;
		allowedDirections[ uint( Direction::LEFT ) ] = true;
		allowedDirections[ uint( Direction::RIGHT ) ] = true;
	}
	if (modeFlags & ModeFlags::HAS_DIRECTION_UD)
	{
		hasAnyDirections = true;
		allowedDirections[ uint( Direction::UP ) ] = true;
		allowedDirections[ uint( Direction::DOWN ) ] = true;
	}
	if (modeFlags & ModeFlags::HAS_DIRECTION_HV)
	{
		hasAnyDirections = true;
		allowedDirections[ uint( Direction::HORIZONTAL ) ] = true;
		allowedDirections[ uint( Direction::VERTICAL ) ] = true;
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
	return int( mode ) >= int( ColorMode::NONE ) && int( mode ) <= int( ColorMode::RANDOM );
}

static bool isValidZoneType( ZoneType type )
{
	return int( type ) >= int( ZoneType::SINGLE ) && int( type ) <= int( ZoneType::MATRIX );
}


//======================================================================================================================
//  message header

void Header::serialize( BufferOutputStream & stream ) const
{
	stream << magic[0] << magic[1] << magic[2] << magic[3];
	stream.writeIntLE( device_idx );
	stream.writeEnumLE( message_type );
	stream.writeIntLE( message_size );
}

bool Header::deserialize( BufferInputStream & stream )
{
	stream >> magic[0] >> magic[1] >> magic[2] >> magic[3];
	stream.readIntLE( device_idx );
	stream.readEnumLE( message_type );
	stream.readIntLE( message_size );

	if (strncmp( magic, "ORGB", sizeof(magic) ) != 0)
		stream.setFailed();
	if (!isValidMessageType( message_type ))
		stream.setFailed();

	return !stream.hasFailed();
}


//======================================================================================================================
//  repeated message sub-sections

size_t ModeDescription::calcSize() const
{
	return sizeofORGBString( name )
	     + sizeof( value )
	     + sizeof( flags )
	     + sizeof( speed_min )
	     + sizeof( speed_max )
	     + sizeof( colors_min )
	     + sizeof( colors_max )
	     + sizeof( speed )
	     + sizeof( direction )
	     + sizeof( color_mode )
	     + sizeofORGBArray( colors );
}

void ModeDescription::serialize( BufferOutputStream & stream ) const
{
	writeORGBString( stream, name );
	stream.writeIntLE( value );
	stream.writeIntLE( flags );
	stream.writeIntLE( speed_min );
	stream.writeIntLE( speed_max );
	stream.writeIntLE( colors_min );
	stream.writeIntLE( colors_max );
	stream.writeIntLE( speed );
	stream.writeEnumLE( direction );
	stream.writeEnumLE( color_mode );
	writeORGBArray( stream, colors );
}

bool ModeDescription::deserialize( BufferInputStream & stream )
{
	readORGBString( stream, name );
	stream.readIntLE( value );
	stream.readIntLE( flags );
	stream.readIntLE( speed_min );
	stream.readIntLE( speed_max );
	stream.readIntLE( colors_min );
	stream.readIntLE( colors_max );
	stream.readIntLE( speed );
	stream.readEnumLE( direction );
	stream.readEnumLE( color_mode );
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
	return sizeofORGBString( name )
	     + sizeof( type )
	     + sizeof( leds_min )
	     + sizeof( leds_max )
	     + sizeof( leds_count )
	     + sizeof( matrix_length )
	     + matrix_length == 0 ? 0 :
	     (
	           sizeof( matrix_height )
	         + sizeof( matrix_width )
	         + sizeofVector( matrix_values )
	     );
}

void ZoneDescription::serialize( BufferOutputStream & stream ) const
{
	writeORGBString( stream, name );
	stream.writeEnumLE( type );
	stream.writeIntLE( leds_min );
	stream.writeIntLE( leds_max );
	stream.writeIntLE( leds_count );
	stream.writeIntLE( matrix_length );
	if (matrix_length > 0)
	{
		stream.writeIntLE( matrix_height );
		stream.writeIntLE( matrix_width );
		for (auto val : matrix_values)
		{
			stream.writeIntLE( val );
		}
	}
}

bool ZoneDescription::deserialize( BufferInputStream & stream)
{
	readORGBString( stream, name );
	stream.readEnumLE( type );
	stream.readIntLE( leds_min );
	stream.readIntLE( leds_max );
	stream.readIntLE( leds_count );
	stream.readIntLE( matrix_length );
	if (matrix_length > 0)
	{
		stream.readIntLE( matrix_height );
		stream.readIntLE( matrix_width );
		size_t matrixSize = matrix_height * matrix_width;
		matrix_values.resize( matrixSize );
		for (size_t i = 0; i < matrixSize; ++i)
		{
			stream.readIntLE( matrix_values[i] );
		}
	}

	if (!isValidZoneType( type ))
		stream.setFailed();

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

size_t LEDDescription::calcSize() const
{
	return sizeofORGBString( name )
	     + sizeof( value );
}

void LEDDescription::serialize( BufferOutputStream & stream ) const
{
	writeORGBString( stream, name );
	stream.writeIntLE( value );
}

bool LEDDescription::deserialize( BufferInputStream & stream )
{
	readORGBString( stream, name );
	stream.readIntLE( value );

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

size_t DeviceDescription::calcSize() const
{
	return sizeof( device_type )
	     + sizeofORGBString( name )
	     + sizeofORGBString( description )
	     + sizeofORGBString( version )
	     + sizeofORGBString( serial )
	     + sizeofORGBString( location )
	     + sizeof( uint16_t )
	     + sizeof( active_mode )
	     + sizeofVector( modes );
}

void DeviceDescription::serialize( BufferOutputStream & stream ) const
{
	stream.writeEnumLE( device_type );
	writeORGBString( stream, name );
	writeORGBString( stream, description );
	writeORGBString( stream, version );
	writeORGBString( stream, serial );
	writeORGBString( stream, location );
	stream.writeIntLE( uint16_t( modes.size() ) );  // the size is not directly before the array, so it must be written manually
	stream.writeIntLE( active_mode );
	for (const ModeDescription & mode : modes)
	{
		mode.serialize( stream );
	}
	writeORGBArray( stream, zones );
	writeORGBArray( stream, leds );
	writeORGBArray( stream, colors );
}

bool DeviceDescription::deserialize( BufferInputStream & stream )
{
	stream.readEnumLE( device_type );
	readORGBString( stream, name );
	readORGBString( stream, description );
	readORGBString( stream, version );
	readORGBString( stream, serial );
	readORGBString( stream, location );
	uint16_t num_modes;
	stream.readIntLE( num_modes );  // the size is not directly before the array, so it must be read manually
	stream.readIntLE( active_mode );
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
//  main protocol messages

uint32_t RequestControllerCount::calcDataSize() const
{
	return 0;
}

void RequestControllerCount::serialize( BufferOutputStream & stream ) const
{
	header.serialize( stream );
}

bool RequestControllerCount::deserializeBody( BufferInputStream & stream )
{
	// don't read the header, the header will be read in advance in order to recognize message type

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t ReplyControllerCount::calcDataSize() const
{
	return uint32_t(
		sizeof( count )
	);
}

void ReplyControllerCount::serialize( BufferOutputStream & stream ) const
{
	header.serialize( stream );

	stream.writeIntLE( count );
}

bool ReplyControllerCount::deserializeBody( BufferInputStream & stream )
{
	// don't read the header, the header will be read in advance in order to recognize message type

	stream.readIntLE( count );

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t RequestControllerData::calcDataSize() const
{
	return 0;
}

void RequestControllerData::serialize( BufferOutputStream & stream ) const
{
	header.serialize( stream );
}

bool RequestControllerData::deserializeBody( BufferInputStream & stream )
{
	// don't read the header, the header will be read in advance in order to recognize message type

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t ReplyControllerData::calcDataSize() const
{
	return uint32_t(
		  sizeof( data_size )
		+ device_desc.calcSize()
	);
}

void ReplyControllerData::serialize( BufferOutputStream & stream ) const
{
	header.serialize( stream );

	stream.writeIntLE( data_size );
	device_desc.serialize( stream );
}

bool ReplyControllerData::deserializeBody( BufferInputStream & stream )
{
	// don't read the header, the header will be read in advance in order to recognize message type

	stream.readIntLE( data_size );
	device_desc.deserialize( stream );

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t SetClientName::calcDataSize() const
{
	return uint32_t(
		name.size() + 1
	);
}

void SetClientName::serialize( BufferOutputStream & stream ) const
{
	header.serialize( stream );

	// TODO: verify in OpenRGB source code
	stream.writeString0( name );
}

bool SetClientName::deserializeBody( BufferInputStream & stream )
{
	// don't read the header, the header will be read in advance in order to recognize message type

	// TODO: verify in OpenRGB source code
	stream.readString0( name );

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t DeviceListUpdated::calcDataSize() const
{
	return 0;
}

void DeviceListUpdated::serialize( BufferOutputStream & stream ) const
{
	header.serialize( stream );
}

bool DeviceListUpdated::deserializeBody( BufferInputStream & stream )
{
	// don't read the header, the header will be read in advance in order to recognize message type

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t ResizeZone::calcDataSize() const
{
	return uint32_t(
		  sizeof( zone_idx )
		+ sizeof( new_size )
	);
}

void ResizeZone::serialize( BufferOutputStream & stream ) const
{
	header.serialize( stream );

	stream.writeIntLE( zone_idx );
	stream.writeIntLE( new_size );
}

bool ResizeZone::deserializeBody( BufferInputStream & stream )
{
	// don't read the header, the header will be read in advance in order to recognize message type

	stream.readIntLE( zone_idx );
	stream.readIntLE( new_size );

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t UpdateLEDs::calcDataSize() const
{
	return uint32_t(
		  sizeof( data_size )
		+ sizeofORGBArray( colors )
	);
}

void UpdateLEDs::serialize( BufferOutputStream & stream ) const
{
	header.serialize( stream );

	stream.writeIntLE( data_size );
	writeORGBArray( stream, colors );
}

bool UpdateLEDs::deserializeBody( BufferInputStream & stream )
{
	// don't read the header, the header will be read in advance in order to recognize message type

	stream.readIntLE( data_size );
	readORGBArray( stream, colors );

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t UpdateZoneLEDs::calcDataSize() const
{
	return uint32_t(
		  sizeof( data_size )
		+ sizeof( zone_idx )
		+ sizeofORGBArray( colors )
	);
}

void UpdateZoneLEDs::serialize( BufferOutputStream & stream ) const
{
	header.serialize( stream );

	stream.writeIntLE( data_size );
	stream.writeIntLE( zone_idx );
	writeORGBArray( stream, colors );
}

bool UpdateZoneLEDs::deserializeBody( BufferInputStream & stream )
{
	// don't read the header, the header will be read in advance in order to recognize message type

	stream.readIntLE( data_size );
	stream.readIntLE( zone_idx );
	readORGBArray( stream, colors );

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t UpdateSingleLED::calcDataSize() const
{
	return uint32_t(
		  sizeof( led_idx )
		+ color.calcSize()
	);
}

void UpdateSingleLED::serialize( BufferOutputStream & stream ) const
{
	header.serialize( stream );

	stream.writeIntLE( led_idx );
	color.serialize( stream );
}

bool UpdateSingleLED::deserializeBody( BufferInputStream & stream )
{
	// don't read the header, the header will be read in advance in order to recognize message type

	stream.readIntLE( led_idx );
	color.deserialize( stream );

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t SetCustomMode::calcDataSize() const
{
	return 0;
}

void SetCustomMode::serialize( BufferOutputStream & stream ) const
{
	header.serialize( stream );
}

bool SetCustomMode::deserializeBody( BufferInputStream & stream )
{
	// don't read the header, the header will be read in advance in order to recognize message type

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t UpdateMode::calcDataSize() const
{
	return uint32_t(
		  sizeof( data_size )
		+ sizeof( mode_idx )
		+ mode_desc.calcSize()
	);
}

void UpdateMode::serialize( BufferOutputStream & stream ) const
{
	header.serialize( stream );

	stream.writeIntLE( data_size );
	stream.writeIntLE( mode_idx );
	mode_desc.serialize( stream );
}

bool UpdateMode::deserializeBody( BufferInputStream & stream )
{
	// don't read the header, the header will be read in advance in order to recognize message type

	stream.readIntLE( data_size );
	stream.readIntLE( mode_idx );
	mode_desc.deserialize( stream );

	return !stream.hasFailed();
}


//======================================================================================================================


} // namespace orgb
