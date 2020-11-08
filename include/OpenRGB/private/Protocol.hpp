//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Created on:  1.11.2020
// Description: declaration of the protocol messages and types
//======================================================================================================================

#ifndef OPENRGB_PROTOCOL_INCLUDED
#define OPENRGB_PROTOCOL_INCLUDED


// declarations from the OpenRGB project
#include "../external/NetworkProtocol.h"
#include "../external/RGBController.h"

#include "OpenRGB/private/Color.hpp"
namespace orgb {
	class BufferOutputStream;
	class BufferInputStream;
}

#include <cstdint>
#include <string>
#include <vector>


namespace orgb {


//======================================================================================================================
//  message header

enum class MessageType : uint32_t
{
	REQUEST_CONTROLLER_COUNT       = NET_PACKET_ID_REQUEST_CONTROLLER_COUNT,
	REQUEST_CONTROLLER_DATA        = NET_PACKET_ID_REQUEST_CONTROLLER_DATA,
	SET_CLIENT_NAME                = NET_PACKET_ID_SET_CLIENT_NAME,
	DEVICE_LIST_UPDATED            = NET_PACKET_ID_DEVICE_LIST_UPDATED,
	RGBCONTROLLER_RESIZEZONE       = NET_PACKET_ID_RGBCONTROLLER_RESIZEZONE,
	RGBCONTROLLER_UPDATELEDS       = NET_PACKET_ID_RGBCONTROLLER_UPDATELEDS,
	RGBCONTROLLER_UPDATEZONELEDS   = NET_PACKET_ID_RGBCONTROLLER_UPDATEZONELEDS,
	RGBCONTROLLER_UPDATESINGLELED  = NET_PACKET_ID_RGBCONTROLLER_UPDATESINGLELED,
	RGBCONTROLLER_SETCUSTOMMODE    = NET_PACKET_ID_RGBCONTROLLER_SETCUSTOMMODE,
	RGBCONTROLLER_UPDATEMODE       = NET_PACKET_ID_RGBCONTROLLER_UPDATEMODE
};

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

	void serialize( BufferOutputStream & stream ) const;
	bool deserialize( BufferInputStream & stream );
};


//======================================================================================================================
//  types

// TODO: strings for all enum values

enum class DeviceType : uint32_t
{
	MOTHERBOARD    = DEVICE_TYPE_MOTHERBOARD,
	DRAM           = DEVICE_TYPE_DRAM,
	GPU            = DEVICE_TYPE_GPU,
	COOLER         = DEVICE_TYPE_COOLER,
	LEDSTRIP       = DEVICE_TYPE_LEDSTRIP,
	KEYBOARD       = DEVICE_TYPE_KEYBOARD,
	MOUSE          = DEVICE_TYPE_MOUSE,
	MOUSEMAT       = DEVICE_TYPE_MOUSEMAT,
	HEADSET        = DEVICE_TYPE_HEADSET,
	HEADSET_STAND  = DEVICE_TYPE_HEADSET_STAND,
	GAMEPAD        = DEVICE_TYPE_GAMEPAD,
	UNKNOWN        = DEVICE_TYPE_UNKNOWN
};

enum ModeFlags : uint32_t
{
	HAS_SPEED                = MODE_FLAG_HAS_SPEED,
	HAS_DIRECTION_LR         = MODE_FLAG_HAS_DIRECTION_LR,
	HAS_DIRECTION_UD         = MODE_FLAG_HAS_DIRECTION_UD,
	HAS_DIRECTION_HV         = MODE_FLAG_HAS_DIRECTION_HV,
	HAS_BRIGHTNESS           = MODE_FLAG_HAS_BRIGHTNESS,
	HAS_PER_LED_COLOR        = MODE_FLAG_HAS_PER_LED_COLOR,
	HAS_MODE_SPECIFIC_COLOR  = MODE_FLAG_HAS_MODE_SPECIFIC_COLOR,
	HAS_RANDOM_COLOR         = MODE_FLAG_HAS_RANDOM_COLOR
};

enum class Direction : uint32_t
{
	LEFT        = MODE_DIRECTION_LEFT,
	RIGHT       = MODE_DIRECTION_RIGHT,
	UP          = MODE_DIRECTION_UP,
	DOWN        = MODE_DIRECTION_DOWN,
	HORIZONTAL  = MODE_DIRECTION_HORIZONTAL,
	VERTICAL    = MODE_DIRECTION_VERTICAL,
};

enum class ColorMode : uint32_t
{
	NONE           = MODE_COLORS_NONE,           // mode has no colors
	PER_LED        = MODE_COLORS_PER_LED,        // mode has per LED colors
	MODE_SPECIFIC  = MODE_COLORS_MODE_SPECIFIC,  // mode specific colors
	RANDOM         = MODE_COLORS_RANDOM,         // mode has random colors
};

enum class ZoneType : uint32_t
{
	SINGLE  = ZONE_TYPE_SINGLE,
	LINEAR  = ZONE_TYPE_LINEAR,
	MATRIX  = ZONE_TYPE_MATRIX
};


//======================================================================================================================
//  repeated message sub-sections

struct ModeDescription
{
	std::string   name;
	uint32_t      value;
	uint32_t      flags;
	uint32_t      speed_min;
	uint32_t      speed_max;
	uint32_t      colors_min;  // TODO: what is this?
	uint32_t      colors_max;  // TODO: what is this?
	uint32_t      speed;
	Direction     direction;
	ColorMode     color_mode;
	std::vector< Color >  colors;

	size_t calcSize() const;
	void serialize( BufferOutputStream & stream ) const;
	bool deserialize( BufferInputStream & stream );
};

struct ZoneDescription
{
	std::string   name;
	ZoneType      type;
	uint32_t      leds_min;    // TODO: what is this?
	uint32_t      leds_max;    // TODO: what is this?
	uint32_t      leds_count;  // TODO: what is this?
	uint16_t      matrix_length;

	// optional
	uint32_t      matrix_height;
	uint32_t      matrix_width;
	std::vector< uint32_t >  matrix_values;  // TODO: what is this?

	size_t calcSize() const;
	void serialize( BufferOutputStream & stream ) const;
	bool deserialize( BufferInputStream & stream );
};

struct LEDDescription
{
	std::string  name;
	uint32_t     value;

	size_t calcSize() const;
	void serialize( BufferOutputStream & stream ) const;
	bool deserialize( BufferInputStream & stream );
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
	void serialize( BufferOutputStream & stream ) const;
	bool deserialize( BufferInputStream & stream );
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
	void serialize( BufferOutputStream & stream ) const;
	bool deserializeBody( BufferInputStream & stream );
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
	void serialize( BufferOutputStream & stream ) const;
	bool deserializeBody( BufferInputStream & stream );
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
	void serialize( BufferOutputStream & stream ) const;
	bool deserializeBody( BufferInputStream & stream );
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
	void serialize( BufferOutputStream & stream ) const;
	bool deserializeBody( BufferInputStream & stream );
};

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
	void serialize( BufferOutputStream & stream ) const;
	bool deserializeBody( BufferInputStream & stream );
};

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
	void serialize( BufferOutputStream & stream ) const;
	bool deserializeBody( BufferInputStream & stream );
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
	void serialize( BufferOutputStream & stream ) const;
	bool deserializeBody( BufferInputStream & stream );
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
	void serialize( BufferOutputStream & stream ) const;
	bool deserializeBody( BufferInputStream & stream );
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
	void serialize( BufferOutputStream & stream ) const;
	bool deserializeBody( BufferInputStream & stream );
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
	void serialize( BufferOutputStream & stream ) const;
	bool deserializeBody( BufferInputStream & stream );
};

// TODO: what is this?
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
	void serialize( BufferOutputStream & stream ) const;
	bool deserializeBody( BufferInputStream & stream );
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
	void serialize( BufferOutputStream & stream ) const;
	bool deserializeBody( BufferInputStream & stream );
};


//======================================================================================================================


} // namespace orgb


#endif // OPENRGB_PROTOCOL_INCLUDED
