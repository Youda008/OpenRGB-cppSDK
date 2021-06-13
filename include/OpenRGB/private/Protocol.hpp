//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
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


/// version of the protocol this client currently implements
/** The following structs and enums are defined according to this version of the protocol.
  * Older versions will not be supported, sorry guys. */
constexpr unsigned int implementedProtocolVersion = 1;


//======================================================================================================================
//  message header

/// a copy of enum in https://gitlab.com/CalcProgrammer1/OpenRGB/-/blob/master/NetworkProtocol.h
enum class MessageType : uint32_t
{
	REQUEST_CONTROLLER_COUNT       = 0,
	REQUEST_CONTROLLER_DATA        = 1,
	REQUEST_PROTOCOL_VERSION       = 40,
	SET_CLIENT_NAME                = 50,
	DEVICE_LIST_UPDATED            = 100,
	RGBCONTROLLER_RESIZEZONE       = 1000,
	RGBCONTROLLER_UPDATELEDS       = 1050,
	RGBCONTROLLER_UPDATEZONELEDS   = 1051,
	RGBCONTROLLER_UPDATESINGLELED  = 1052,
	RGBCONTROLLER_SETCUSTOMMODE    = 1100,
	RGBCONTROLLER_UPDATEMODE       = 1101,
};
const char * enumString( MessageType );

/** Every protocol message starts with this. */
struct Header
{
	char         magic [4];  ///< must always be set to ORGB in all messages
	uint32_t     device_idx;
	MessageType  message_type;
	uint32_t     message_size;  ///< size of message minus size of this header

	Header() {}
	Header( MessageType messageType )
		: magic{'O','R','G','B'}, device_idx(0), message_type( messageType ) {}
	Header( MessageType messageType, uint32_t messageSize )
		: magic{'O','R','G','B'}, device_idx(0), message_type( messageType ), message_size( messageSize ) {}
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
	Motherboard   = 0,
	DRAM          = 1,
	GPU           = 2,
	Cooler        = 3,
	LedStrip      = 4,
	Keyboard      = 5,
	Mouse         = 6,
	MouseMat      = 7,
	Headset       = 8,
	HeadsetStand  = 9,
	Gamepad       = 10,
	Unknown       = 11,
};
const char * enumString( DeviceType );

/** Which features the mode supports */
enum ModeFlags : uint32_t
{
	HasSpeed              = (1 << 0),  // the speed attribute in ModeDescription is present
	HasDirectionLR        = (1 << 1),  // the direction attribute in ModeDescription can have LEFT or RIGHT values
	HasDirectionUD        = (1 << 2),  // the direction attribute in ModeDescription can have UP or DOWN values
	HasDirectionHV        = (1 << 3),  // the direction attribute in ModeDescription can have HORIZONTAL or VERTICAL values
	HasBrightness         = (1 << 4),  // the brightness attribute in ModeDescription is present
	HasPerLedColor        = (1 << 5),  // the color_mode attribute in ModeDescription can be set to PER_LED
	HasModeSpecificColor  = (1 << 6),  // the color_mode attribute in ModeDescription can be set to MODE_SPECIFIC
	HasRandomColor        = (1 << 7),  // the color_mode attribute in ModeDescription can be set to RANDOM
};
std::string modeFlagsToString( uint32_t flags );

/** Direction of the color effect */
enum class Direction : uint32_t
{
	Left        = 0,
	Right       = 1,
	Up          = 2,
	Down        = 3,
	Horizontal  = 4,
	Vertical    = 5
};
const char * enumString( Direction );

/** How the colors of a mode are set */
enum class ColorMode : uint32_t
{
	None          = 0,  // mode has no colors
	PerLed        = 1,  // mode has per LED colors
	ModeSpecific  = 2,  // mode specific colors
	Random        = 3   // mode has random colors
};
const char * enumString( ColorMode );

/** Type of RGB zone */
enum class ZoneType : uint32_t
{
	Single  = 0,
	Linear  = 1,
	Matrix  = 2
};
const char * enumString( ZoneType );


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
	std::string   vendor;
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

/** Asks server how many RGB devices (controllers) there are. */
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
	/** The header must be deserialized separately prior to this, in order to determine the type of message and
	  * which object to construct and fill up. */
	bool deserializeBody( own::BufferInputStream & stream );
};

/** A reply to RequestControllerCount */
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
	/** The header must be deserialized separately prior to this, in order to determine the type of message and
	  * which object to construct and fill up. */
	bool deserializeBody( own::BufferInputStream & stream );
};

/** Asks for all information and supported modes about a specific RGB device (controller). */
struct RequestControllerData
{
	Header header = {
		/*message_type*/ thisType,
		/*message_size*/ 4
	};
	uint32_t protocolVersion;

 // support for templated processing

	static constexpr MessageType thisType = MessageType::REQUEST_CONTROLLER_DATA;

	RequestControllerData() {}
	RequestControllerData( uint32_t deviceIdx, uint32_t protocolVersion )
	{
		header.device_idx = deviceIdx;
		this->protocolVersion = protocolVersion;
	}

	uint32_t calcDataSize() const;
	void serialize( own::BufferOutputStream & stream ) const;
	/** The header must be deserialized separately prior to this, in order to determine the type of message and
	  * which object to construct and fill up. */
	bool deserializeBody( own::BufferInputStream & stream );
};

/** A reply to RequestControllerData */
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
	/** The header must be deserialized separately prior to this, in order to determine the type of message and
	  * which object to construct and fill up. */
	bool deserializeBody( own::BufferInputStream & stream );
};

/** Tells the server in what version of the protocol the client wants to communite in. */
struct RequestProtocolVersion
{
	Header header = {
		/*device_idx*/   0,
		/*message_type*/ thisType,
		/*message_size*/ 4
	};
	uint32_t clientVersion;

 // support for templated processing

	static constexpr MessageType thisType = MessageType::REQUEST_PROTOCOL_VERSION;

	RequestProtocolVersion()
	{
		clientVersion = implementedProtocolVersion;
	}

	uint32_t calcDataSize() const;
	void serialize( own::BufferOutputStream & stream ) const;
	/** The header must be deserialized separately prior to this, in order to determine the type of message and
	  * which object to construct and fill up. */
	bool deserializeBody( own::BufferInputStream & stream );
};

/** A reply to RequestProtocolVersion. Contains the maximum version the server supports. */
struct ReplyProtocolVersion
{
	Header header = {
		/*device_idx*/   0,
		/*message_type*/ thisType,
		/*message_size*/ 4
	};
	uint32_t serverVersion;

 // support for templated processing

	static constexpr MessageType thisType = MessageType::REQUEST_PROTOCOL_VERSION;

	ReplyProtocolVersion() {}
	ReplyProtocolVersion( uint32_t protocolVersion )
	{
		serverVersion = protocolVersion;
	}

	uint32_t calcDataSize() const;
	void serialize( own::BufferOutputStream & stream ) const;
	/** The header must be deserialized separately prior to this, in order to determine the type of message and
	  * which object to construct and fill up. */
	bool deserializeBody( own::BufferInputStream & stream );
};

/** Announces a custom name of the client to the server. */
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
	/** The header must be deserialized separately prior to this, in order to determine the type of message and
	  * which object to construct and fill up. */
	bool deserializeBody( own::BufferInputStream & stream );
};

/** This is sent from the server everytime its device list has changed. */
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
	/** The header must be deserialized separately prior to this, in order to determine the type of message and
	  * which object to construct and fill up. */
	bool deserializeBody( own::BufferInputStream & stream );
};

/** Resizes a zone of LEDs, if the device supports it. */
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
	/** The header must be deserialized separately prior to this, in order to determine the type of message and
	  * which object to construct and fill up. */
	bool deserializeBody( own::BufferInputStream & stream );
};

/** Applies individually selected color to every LED. */
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
	/** The header must be deserialized separately prior to this, in order to determine the type of message and
	  * which object to construct and fill up. */
	bool deserializeBody( own::BufferInputStream & stream );
};

/** Applies individually selected color to every LED in a specific zone. */
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
	/** The header must be deserialized separately prior to this, in order to determine the type of message and
	  * which object to construct and fill up. */
	bool deserializeBody( own::BufferInputStream & stream );
};

/** Changes color of a single particular LED. */
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
	/** The header must be deserialized separately prior to this, in order to determine the type of message and
	  * which object to construct and fill up. */
	bool deserializeBody( own::BufferInputStream & stream );
};

/** Switches mode of a device to "Direct" mode */
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
	/** The header must be deserialized separately prior to this, in order to determine the type of message and
	  * which object to construct and fill up. */
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
	/** The header must be deserialized separately prior to this, in order to determine the type of message and
	  * which object to construct and fill up. */
	bool deserializeBody( own::BufferInputStream & stream );
};


//======================================================================================================================


} // namespace orgb


#endif // OPENRGB_PROTOCOL_INCLUDED
