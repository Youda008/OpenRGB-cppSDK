//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Created on:  1.11.2020
// Description: declaration of the protocol messages and types
//======================================================================================================================

#ifndef OPENRGB_PROTOCOL_INCLUDED
#define OPENRGB_PROTOCOL_INCLUDED


#include "OpenRGB/Color.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace own {
	class BufferOutputStream;
	class BufferInputStream;
}


namespace orgb {


//======================================================================================================================
//  message header

enum class MessageType : uint32_t
{
	REQUEST_CONTROLLER_COUNT       = 0,
	REQUEST_CONTROLLER_DATA        = 1,
	SET_CLIENT_NAME                = 50,
	DEVICE_LIST_UPDATED            = 100,
	RGBCONTROLLER_RESIZEZONE       = 1000,
	RGBCONTROLLER_UPDATELEDS       = 1050,
	RGBCONTROLLER_UPDATEZONELEDS   = 1051,
	RGBCONTROLLER_UPDATESINGLELED  = 1052,
	RGBCONTROLLER_SETCUSTOMMODE    = 1100,
	RGBCONTROLLER_UPDATEMODE       = 1101,
};
const char * toString( MessageType );

struct Header
{
	char         magic [4];  ///< must always be set to ORGB in all messages
	uint32_t     device_idx;
	MessageType  message_type;
	uint32_t     message_size;  ///< size of message minus size of this header

	Header() {}
	Header( MessageType messageType )
		: magic{'O','R','G','B'}, message_type( messageType ) {}
	Header( MessageType messageType, uint32_t messageSize )
		: magic{'O','R','G','B'}, message_type( messageType ), message_size( messageSize ) {}
	Header( uint32_t deviceIdx, MessageType messageType )
		: magic{'O','R','G','B'}, device_idx( deviceIdx ), message_type( messageType ) {}
	Header( uint32_t deviceIdx, MessageType messageType, uint32_t messageSize )
		: magic{'O','R','G','B'}, device_idx( deviceIdx ), message_type( messageType ), message_size( messageSize ) {}

	static constexpr size_t size() { return sizeof(Header); }  // all members are equally big, no padding will take place

	void serialize( own::BufferOutputStream & stream ) const;
	bool deserialize( own::BufferInputStream & stream );
};


//======================================================================================================================
//  types

/** Type of device with RGB LEDs */
enum class DeviceType : uint32_t
{
	MOTHERBOARD   = 0,
	DRAM          = 1,
	GPU           = 2,
	COOLER        = 3,
	LEDSTRIP      = 4,
	KEYBOARD      = 5,
	MOUSE         = 6,
	MOUSEMAT      = 7,
	HEADSET       = 8,
	HEADSET_STAND = 9,
	GAMEPAD       = 10,
	UNKNOWN       = 11,
};
const char * toString( DeviceType );

/** Which features the mode supports */
enum ModeFlags : uint32_t
{
	HAS_SPEED                = (1 << 0),  // the speed attribute in ModeDescription is present
	HAS_DIRECTION_LR         = (1 << 1),  // the direction attribute in ModeDescription can have LEFT or RIGHT values
	HAS_DIRECTION_UD         = (1 << 2),  // the direction attribute in ModeDescription can have UP or DOWN values
	HAS_DIRECTION_HV         = (1 << 3),  // the direction attribute in ModeDescription can have HORIZONTAL or VERTICAL values
	HAS_BRIGHTNESS           = (1 << 4),  // the brightness attribute in ModeDescription is present
	HAS_PER_LED_COLOR        = (1 << 5),  // the color_mode attribute in ModeDescription can be set to PER_LED
	HAS_MODE_SPECIFIC_COLOR  = (1 << 6),  // the color_mode attribute in ModeDescription can be set to MODE_SPECIFIC
	HAS_RANDOM_COLOR         = (1 << 7),  // the color_mode attribute in ModeDescription can be set to RANDOM
};
std::string modeFlagsToString( uint32_t flags );

/** Direction of the color effect */
enum class Direction : uint32_t
{
	LEFT        = 0,
	RIGHT       = 1,
	UP          = 2,
	DOWN        = 3,
	HORIZONTAL  = 4,
	VERTICAL    = 5
};
const char * toString( Direction );

/** How the colors of a mode are set */
enum class ColorMode : uint32_t
{
	NONE           = 0,  // mode has no colors
	PER_LED        = 1,  // mode has per LED colors
	MODE_SPECIFIC  = 2,  // mode specific colors
	RANDOM         = 3   // mode has random colors
};
const char * toString( ColorMode );

/** Type of RGB zone */
enum class ZoneType : uint32_t
{
	SINGLE  = 0,
	LINEAR  = 1,
	MATRIX  = 2
};
const char * toString( ZoneType );


//======================================================================================================================
//  repeated message sub-sections

struct ModeDescription
{
	std::string   name;
	uint32_t      value;
	uint32_t      flags;
	uint32_t      speed_min;
	uint32_t      speed_max;
	uint32_t      colors_min;
	uint32_t      colors_max;
	uint32_t      speed;
	Direction     direction;
	ColorMode     color_mode;
	std::vector< Color >  colors;

	size_t calcSize() const;
	void serialize( own::BufferOutputStream & stream ) const;
	bool deserialize( own::BufferInputStream & stream );
};

struct ZoneDescription
{
	std::string   name;
	ZoneType      type;
	uint32_t      leds_min;
	uint32_t      leds_max;
	uint32_t      leds_count;
	uint16_t      matrix_length;

	// optional
	uint32_t      matrix_height;
	uint32_t      matrix_width;
	std::vector< uint32_t >  matrix_values;

	size_t calcSize() const;
	void serialize( own::BufferOutputStream & stream ) const;
	bool deserialize( own::BufferInputStream & stream );
};

struct LEDDescription
{
	std::string  name;
	uint32_t     value;

	size_t calcSize() const;
	void serialize( own::BufferOutputStream & stream ) const;
	bool deserialize( own::BufferInputStream & stream );
};

struct DeviceDescription
{
	DeviceType    device_type;
	std::string   name;
	std::string   description;
	std::string   version;
	std::string   serial;
	std::string   location;
	uint32_t      active_mode;
	std::vector< ModeDescription >  modes;
	std::vector< ZoneDescription >  zones;
	std::vector< LEDDescription >   leds;
	std::vector< Color >            colors;

	size_t calcSize() const;
	void serialize( own::BufferOutputStream & stream ) const;
	bool deserialize( own::BufferInputStream & stream );
};


//======================================================================================================================
//  main protocol messages

struct RequestControllerCount
{
	Header header = {
		/*device_idx*/   0,
		/*message_type*/ thisType,
		/*message_size*/ 0
	};

 // support for templated processing

	static constexpr MessageType thisType = MessageType::REQUEST_CONTROLLER_COUNT;

	RequestControllerCount() {}

	uint32_t calcDataSize() const;
	void serialize( own::BufferOutputStream & stream ) const;
	bool deserializeBody( own::BufferInputStream & stream );
};

struct ReplyControllerCount
{
	Header header = {
		/*device_idx*/   0,
		/*message_type*/ thisType,
		/*message_size*/ 4
	};
	uint32_t count;

 // support for templated processing

	static constexpr MessageType thisType = MessageType::REQUEST_CONTROLLER_COUNT;

	ReplyControllerCount() {}
	ReplyControllerCount( uint32_t count )
	:
		count( count )
	{}

	uint32_t calcDataSize() const;
	void serialize( own::BufferOutputStream & stream ) const;
	bool deserializeBody( own::BufferInputStream & stream );
};

struct RequestControllerData
{
	Header header = {
		/*message_type*/ thisType,
		/*message_size*/ 0
	};

 // support for templated processing

	static constexpr MessageType thisType = MessageType::REQUEST_CONTROLLER_DATA;

	RequestControllerData() {}
	RequestControllerData( uint32_t deviceIdx )
	{
		header.device_idx = deviceIdx;
	}

	uint32_t calcDataSize() const;
	void serialize( own::BufferOutputStream & stream ) const;
	bool deserializeBody( own::BufferInputStream & stream );
};

struct ReplyControllerData
{
	Header header = {
		/*message_type*/ thisType
	};
	uint32_t           data_size;  ///< must always be same as header.message_size, no idea why it's there twice
	DeviceDescription  device_desc;

 // support for templated processing

	static constexpr MessageType thisType = MessageType::REQUEST_CONTROLLER_DATA;

	ReplyControllerData() {}
	ReplyControllerData( uint32_t deviceIdx, DeviceDescription && device )
	:
		device_desc( std::move(device) )
	{
		header.device_idx = deviceIdx;
		header.message_size = data_size = calcDataSize();
	}

	uint32_t calcDataSize() const;
	void serialize( own::BufferOutputStream & stream ) const;
	bool deserializeBody( own::BufferInputStream & stream );
};

// sends custom client name when connected
struct SetClientName
{
	Header header = {
		/*device_idx*/   0,
		/*message_type*/ thisType
	};
	std::string  name;

 // support for templated processing

	static constexpr MessageType thisType = MessageType::SET_CLIENT_NAME;

	SetClientName() {}
	SetClientName( const std::string & name )
	:
		name( name )
	{
		header.message_size = calcDataSize();
	}

	uint32_t calcDataSize() const;
	void serialize( own::BufferOutputStream & stream ) const;
	bool deserializeBody( own::BufferInputStream & stream );
};

// this is sent back from the server everytime its device list changes
struct DeviceListUpdated
{
	Header header = {
		/*device_idx*/   0,
		/*message_type*/ thisType,
		/*message_size*/ 0
	};

 // support for templated processing

	static constexpr MessageType thisType = MessageType::DEVICE_LIST_UPDATED;

	DeviceListUpdated() {}

	uint32_t calcDataSize() const;
	void serialize( own::BufferOutputStream & stream ) const;
	bool deserializeBody( own::BufferInputStream & stream );
};

struct ResizeZone
{
	Header header = {
		/*message_type*/ thisType,
		/*message_size*/ 8
	};
	uint32_t  zone_idx;
	uint32_t  new_size;

 // support for templated processing

	static constexpr MessageType thisType = MessageType::RGBCONTROLLER_RESIZEZONE;

	ResizeZone() {}
	ResizeZone( uint32_t deviceIdx, uint32_t zoneIdx, uint32_t newSize )
	:
		zone_idx( zoneIdx ),
		new_size( newSize )
	{
		header.device_idx = deviceIdx;
	}

	uint32_t calcDataSize() const;
	void serialize( own::BufferOutputStream & stream ) const;
	bool deserializeBody( own::BufferInputStream & stream );
};

struct UpdateLEDs
{
	Header header = {
		/*message_type*/ thisType,
	};
	uint32_t  data_size;
	std::vector< Color >  colors;

 // support for templated processing

	static constexpr MessageType thisType = MessageType::RGBCONTROLLER_UPDATELEDS;

	UpdateLEDs() {}
	UpdateLEDs( uint32_t deviceIdx, const std::vector< Color > & colors )
	:
		colors( colors )
	{
		header.device_idx = deviceIdx;
		header.message_size = data_size = calcDataSize();
	}

	uint32_t calcDataSize() const;
	void serialize( own::BufferOutputStream & stream ) const;
	bool deserializeBody( own::BufferInputStream & stream );
};

struct UpdateZoneLEDs
{
	Header header = {
		/*message_type*/ thisType,
	};
	uint32_t  data_size;
	uint32_t  zone_idx;
	std::vector< Color >  colors;

 // support for templated processing

	static constexpr MessageType thisType = MessageType::RGBCONTROLLER_UPDATEZONELEDS;

	UpdateZoneLEDs() {}
	UpdateZoneLEDs( uint32_t deviceIdx, uint32_t zoneIdx, const std::vector< Color > & colors )
	:
		zone_idx( zoneIdx ),
		colors( colors )
	{
		header.device_idx = deviceIdx;
		header.message_size = data_size = calcDataSize();
	}

	uint32_t calcDataSize() const;
	void serialize( own::BufferOutputStream & stream ) const;
	bool deserializeBody( own::BufferInputStream & stream );
};

struct UpdateSingleLED
{
	Header header = {
		/*message_type*/ thisType,
		/*message_size*/ 8
	};
	uint32_t  led_idx;
	Color     color;

 // support for templated processing

	static constexpr MessageType thisType = MessageType::RGBCONTROLLER_UPDATESINGLELED;

	UpdateSingleLED() {}
	UpdateSingleLED( uint32_t deviceIdx, uint32_t ledIdx, Color color )
	:
		led_idx( ledIdx ),
		color( color )
	{
		header.device_idx = deviceIdx;
	}

	uint32_t calcDataSize() const;
	void serialize( own::BufferOutputStream & stream ) const;
	bool deserializeBody( own::BufferInputStream & stream );
};

// switches device mode to direct
struct SetCustomMode
{
	Header header = {
		/*message_type*/ thisType,
		/*message_size*/ 0
	};

 // support for templated processing

	static constexpr MessageType thisType = MessageType::RGBCONTROLLER_SETCUSTOMMODE;

	SetCustomMode();
	SetCustomMode( uint32_t deviceIdx )
	{
		header.device_idx = deviceIdx;
	}

	uint32_t calcDataSize() const;
	void serialize( own::BufferOutputStream & stream ) const;
	bool deserializeBody( own::BufferInputStream & stream );
};

// TODO: what does this mean? how to set active mode?
struct UpdateMode
{
	Header header = {
		/*message_type*/ thisType,
	};
	uint32_t         data_size;
	uint32_t         mode_idx;
	ModeDescription  mode_desc;

 // support for templated processing

	static constexpr MessageType thisType = MessageType::RGBCONTROLLER_UPDATEMODE;

	UpdateMode() {}
	UpdateMode( uint32_t deviceIdx, uint32_t modeIdx, const ModeDescription & modeDesc )
	:
		mode_idx( modeIdx ),
		mode_desc( modeDesc )
	{
		header.device_idx = deviceIdx;
		header.message_size = data_size = calcDataSize();
	}

	uint32_t calcDataSize() const;
	void serialize( own::BufferOutputStream & stream ) const;
	bool deserializeBody( own::BufferInputStream & stream );
};


//======================================================================================================================


} // namespace orgb


#endif // OPENRGB_PROTOCOL_INCLUDED
