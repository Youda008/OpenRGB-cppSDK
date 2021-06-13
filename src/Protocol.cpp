//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: declaration of the protocol messages and types
//======================================================================================================================

#include "OpenRGB/private/Protocol.hpp"

#include "MiscUtils.hpp"
#include "BufferStream.hpp"
using own::BufferOutputStream;
using own::BufferInputStream;
MAKE_LITTLE_ENDIAN_DEFAULT

#include <cstring>  // strncmp
#include <string>
using std::string;
#include <vector>
using std::vector;
#include <sstream>
using std::ostringstream;  // flags to string


namespace orgb {


//======================================================================================================================
//  OpenRGB strings are made of 2 bytes of little-endian length and the string itself, terminated by '\0' character

static size_t sizeofORGBString( const string & str )
{
	return 2 + str.size() + 1;
}

static void writeORGBString( BufferOutputStream & stream, const string & str )
{
	stream << uint16_t(str.size());
	stream.writeString0( str );
}

static bool readORGBString( BufferInputStream & stream, string & str )
{
	uint16_t size = 0;
	stream >> size;
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
	stream << uint16_t(vec.size());
	for (const Type & elem : vec)
	{
		stream << elem;
	}
}

template< typename Type, typename std::enable_if< !std::is_integral<Type>::value, int >::type = 0 >
static void writeORGBArray( BufferOutputStream & stream, const vector< Type > vec )
{
	stream << uint16_t(vec.size());
	for (const Type & elem : vec)
	{
		elem.serialize( stream );
	}
}

template< typename Type, typename std::enable_if< std::is_integral<Type>::value, int >::type = 0 >
static bool readORGBArray( BufferInputStream & stream, vector< Type > & vec )
{
	uint16_t size = 0;
	stream >> size;
	vec.resize( size );
	for (uint16_t i = 0; i < size; ++i)
	{
		stream >> vec[i];
	}
	return !stream.hasFailed();
}

template< typename Type, typename std::enable_if< !std::is_integral<Type>::value, int >::type = 0 >
static bool readORGBArray( BufferInputStream & stream, vector< Type > & vec )
{
	uint16_t size = 0;
	stream >> size;
	vec.resize( size );
	for (uint16_t i = 0; i < size; ++i)
	{
		vec[i].deserialize( stream );
	}
	return !stream.hasFailed();
}


//======================================================================================================================
//  enum strings

const char * enumString( MessageType type )
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
//  message header

void Header::serialize( BufferOutputStream & stream ) const
{
	stream << magic[0] << magic[1] << magic[2] << magic[3];
	stream << device_idx;
	stream << message_type;
	stream << message_size;
}

bool Header::deserialize( BufferInputStream & stream )
{
	stream >> magic[0] >> magic[1] >> magic[2] >> magic[3];
	stream >> device_idx;
	stream >> message_type;
	stream >> message_size;

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

bool ModeDescription::deserialize( BufferInputStream & stream )
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

bool ZoneDescription::deserialize( BufferInputStream & stream)
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
	return sizeofORGBString( name )
	     + sizeof( value );
}

void LEDDescription::serialize( BufferOutputStream & stream ) const
{
	writeORGBString( stream, name );
	stream << value;
}

bool LEDDescription::deserialize( BufferInputStream & stream )
{
	readORGBString( stream, name );
	stream >> value;

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

size_t DeviceDescription::calcSize() const
{
	return sizeof( device_type )
	     + sizeofORGBString( name )
	     + sizeofORGBString( vendor )
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

bool DeviceDescription::deserialize( BufferInputStream & stream )
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

	stream << count;
}

bool ReplyControllerCount::deserializeBody( BufferInputStream & stream )
{
	stream >> count;

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

	stream << data_size;
	device_desc.serialize( stream );
}

bool ReplyControllerData::deserializeBody( BufferInputStream & stream )
{
	stream >> data_size;
	device_desc.deserialize( stream );

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t RequestProtocolVersion::calcDataSize() const
{
	return sizeof( clientVersion );
}

void RequestProtocolVersion::serialize( BufferOutputStream & stream ) const
{
	header.serialize( stream );

	stream << clientVersion;
}

bool RequestProtocolVersion::deserializeBody( BufferInputStream & stream )
{
	stream >> clientVersion;

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t ReplyProtocolVersion::calcDataSize() const
{
	return sizeof( serverVersion );
}

void ReplyProtocolVersion::serialize( BufferOutputStream & stream ) const
{
	header.serialize( stream );

	stream << serverVersion;
}

bool ReplyProtocolVersion::deserializeBody( BufferInputStream & stream )
{
	stream >> serverVersion;

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

	stream.writeString0( name );
}

bool SetClientName::deserializeBody( BufferInputStream & stream )
{
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

	stream << zone_idx;
	stream << new_size;
}

bool ResizeZone::deserializeBody( BufferInputStream & stream )
{
	stream >> zone_idx;
	stream >> new_size;

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

	stream << data_size;
	writeORGBArray( stream, colors );
}

bool UpdateLEDs::deserializeBody( BufferInputStream & stream )
{
	stream >> data_size;
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

	stream << data_size;
	stream << zone_idx;
	writeORGBArray( stream, colors );
}

bool UpdateZoneLEDs::deserializeBody( BufferInputStream & stream )
{
	stream >> data_size;
	stream >> zone_idx;
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

	stream << led_idx;
	color.serialize( stream );
}

bool UpdateSingleLED::deserializeBody( BufferInputStream & stream )
{
	stream >> led_idx;
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

	stream << data_size;
	stream << mode_idx;
	mode_desc.serialize( stream );
}

bool UpdateMode::deserializeBody( BufferInputStream & stream )
{
	stream >> data_size;
	stream >> mode_idx;
	mode_desc.deserialize( stream );

	return !stream.hasFailed();
}


//======================================================================================================================


} // namespace orgb
