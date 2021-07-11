//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: data structures containing device information
//======================================================================================================================

#ifndef OPENRGB_DEVICE_INCLUDED
#define OPENRGB_DEVICE_INCLUDED


#include "OpenRGB/Exceptions.hpp"

#include "Color.hpp"

#include <string>
#include <vector>
#include <memory>  // unique_ptr<Device>


namespace orgb {


class Device;


//======================================================================================================================
//  enums

/// Type of device with RGB LEDs
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
const char * enumString( DeviceType ) noexcept;

/// Which features the mode supports
enum ModeFlags : uint32_t
{
	HasSpeed              = (1 << 0),  ///< the speed attribute in ModeDescription is present
	HasDirectionLR        = (1 << 1),  ///< the direction attribute in ModeDescription can have LEFT or RIGHT values
	HasDirectionUD        = (1 << 2),  ///< the direction attribute in ModeDescription can have UP or DOWN values
	HasDirectionHV        = (1 << 3),  ///< the direction attribute in ModeDescription can have HORIZONTAL or VERTICAL values
	HasBrightness         = (1 << 4),  ///< the brightness attribute in ModeDescription is present
	HasPerLedColor        = (1 << 5),  ///< the color_mode attribute in ModeDescription can be set to PER_LED
	HasModeSpecificColor  = (1 << 6),  ///< the color_mode attribute in ModeDescription can be set to MODE_SPECIFIC
	HasRandomColor        = (1 << 7),  ///< the color_mode attribute in ModeDescription can be set to RANDOM
};
std::string modeFlagsToString( uint32_t flags );

/// Direction of the color effect
enum class Direction : uint32_t
{
	Left        = 0,
	Right       = 1,
	Up          = 2,
	Down        = 3,
	Horizontal  = 4,
	Vertical    = 5,
};
const char * enumString( Direction ) noexcept;

/// How the colors of a mode are set
enum class ColorMode : uint32_t
{
	None          = 0,  ///< mode has no colors
	PerLed        = 1,  ///< mode has per LED colors
	ModeSpecific  = 2,  ///< mode specific colors
	Random        = 3,  ///< mode has random colors
};
const char * enumString( ColorMode ) noexcept;

/// Type of RGB zone
enum class ZoneType : uint32_t
{
	Single  = 0,
	Linear  = 1,
	Matrix  = 2,
};
const char * enumString( ZoneType ) noexcept;


//======================================================================================================================
//  parts of messages used by the network protocol

/// Description of a device's color mode
struct ModeDescription
{
	std::string   name;
	uint32_t      value;       ///< device-specific value
	uint32_t      flags;       ///< see ModeFlags for possible bit flags
	uint32_t      speed_min;   ///< minimum speed value, this attribute is valid only if ModeFlags::HAS_SPEED is set, otherwise it's uninitialized
	uint32_t      speed_max;   ///< maximum speed value, this attribute is valid only if ModeFlags::HAS_SPEED is set, otherwise it's uninitialized
	uint32_t      colors_min;  ///< minimum number of mode colors
	uint32_t      colors_max;  ///< maximum number of mode colors
	uint32_t      speed;       ///< speed of the effect, this attribute is valid only if ModeFlags::HAS_SPEED is set, otherwise it's uninitialized
	Direction     direction;   ///< direction of the color effect, this attribute is only valid if any of ModeFlags::HAS_DIRECTION_XY is set, otherwise it's uninitialized
	ColorMode     color_mode;  ///< how the colors of a mode are set
	std::vector< Color >  colors;  ///< mode-specific list of colors

	size_t calcSize() const noexcept;
	void serialize( own::BinaryOutputStream & stream ) const;
	bool deserialize( own::BinaryInputStream & stream ) noexcept;
};

/// Description of a device's zone
struct ZoneDescription
{
	std::string   name;
	ZoneType      type;
	uint32_t      leds_min;       ///< minimum size of the zone
	uint32_t      leds_max;       ///< maximum size of the zone
	uint32_t      leds_count;     ///< current size of the zone

	// optional
	uint32_t      matrix_height;  ///< if the zone type is matrix, this is its height
	uint32_t      matrix_width;   ///< if the zone type is matrix, this is its width
	std::vector< uint32_t >  matrix_values;  ///< TODO: what is this?

	size_t calcSize() const noexcept;
	void serialize( own::BinaryOutputStream & stream ) const;
	bool deserialize( own::BinaryInputStream & stream ) noexcept;
};

/// Description of a LED
struct LEDDescription
{
	std::string  name;
	uint32_t     value;  ///< device-specific value

	size_t calcSize() const noexcept;
	void serialize( own::BinaryOutputStream & stream ) const;
	bool deserialize( own::BinaryInputStream & stream ) noexcept;
};

/// Description of an RGB device (controller)
struct DeviceDescription
{
	DeviceType    type;
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

	size_t calcSize() const noexcept;
	void serialize( own::BinaryOutputStream & stream ) const;
	bool deserialize( own::BinaryInputStream & stream ) noexcept;
};


//======================================================================================================================
/// Represents a particular LED on an RGB device.

class LED
{

 public:

	const Device & parent;  ///< Warning: This is non-owning reference!! It will cease to be valid when the DeviceList is destructed

	uint32_t      idx;
	std::string   name;
	uint32_t      value;  ///< device-specific value

 public:

	LED( const Device & parent, uint32_t idx, LEDDescription && desc ) noexcept;

	// Must be copyable so that vector can reallocate.
	LED( const LED & other ) = default;
	LED( LED && other ) noexcept = default;

	LED & operator=( const LED & other ) = delete;
	LED & operator=( LED && other ) = delete;

};


//======================================================================================================================
/// Represents a group of LEDs on an RGB device. Only some devices have zones.

class Zone
{

 public:

	const Device & parent;  ///< Warning: This is non-owning reference!! It will cease to be valid when the DeviceList is destructed

	uint32_t      idx;
	std::string   name;
	ZoneType      type;
	uint32_t      minLeds;      ///< minimum size of the zone
	uint32_t      maxLeds;      ///< maximum size of the zone
	uint32_t      numLeds;      ///< current size of the zone
	uint32_t      matrixHeight; ///< if the zone type is matrix, this is its height
	uint32_t      matrixWidth;  ///< if the zone type is matrix, this is its width
	// TODO: the matrix values

 public:

	Zone( const Device & parent, uint32_t idx, ZoneDescription && desc ) noexcept;

	// Must be copyable so that vector can reallocate.
	Zone( const Zone & other ) = default;
	Zone( Zone && other ) noexcept = default;

	Zone & operator=( const Zone & other ) = delete;
	Zone & operator=( Zone && other ) = delete;

};


//======================================================================================================================
/// Represents a color mode of an RGB device, like "breathing", "flashing", "rainbow" or "direct".

class Mode
{

 public:

	const Device & parent;  ///< Warning: This is non-owning reference!! It will cease to be valid when the DeviceList is destructed

	const uint32_t      idx;
	const std::string   name;
	const uint32_t      value;      ///< device-specific value
	const uint32_t      flags;      ///< see ModeFlags for possible bit flags
	      Direction     direction;  ///< direction of the color effect, this attribute is only valid if any of ModeFlags::HAS_DIRECTION_XY is set, otherwise it's uninitialized
	const uint32_t      minSpeed;   ///< this attribute is valid only if ModeFlags::HAS_SPEED is set, otherwise it's uninitialized
	const uint32_t      maxSpeed;   ///< this attribute is valid only if ModeFlags::HAS_SPEED is set, otherwise it's uninitialized
	      uint32_t      speed;      ///< this attribute is valid only if ModeFlags::HAS_SPEED is set, otherwise it's uninitialized
	const uint32_t      minColors;  ///< TODO: what is this?
	const uint32_t      maxColors;  ///< TODO: what is this?
	      ColorMode     colorMode;  ///< how the colors of a mode are set
	      std::vector< Color > colors;

 public:

	Mode( const Device & parent, uint32_t idx, ModeDescription && desc ) noexcept;

	// must be copyable so that vector can reallocate
	Mode( const Mode & other ) = default;

	Mode & operator=( const Mode & other ) = delete;
	Mode & operator=( Mode && other ) = delete;

	void toProtocolDescription( ModeDescription & desc ) const;

};


//======================================================================================================================
/// Represents an RGB-capable device. Device can have modes, zones and individual LEDs.

class Device
{

 public:

	uint32_t idx;
	DeviceType type;
	std::string name;
	std::string vendor;
	std::string description;
	std::string version;
	std::string serial;
	std::string location;
	uint32_t activeMode;

	std::vector< Mode > modes;
	std::vector< Zone > zones;
	std::vector< LED > leds;
	std::vector< Color > colors;

 public:

	Device( uint32_t idx, DeviceDescription && descr );

	// No copies or moves allowed, because they would break the non-owning references in Modes, Zones and LEDs.
	Device( const Device & other ) = delete;
	Device( Device && other ) noexcept = delete;
	Device & operator=( const Device & other ) = delete;
	Device & operator=( Device && other ) = delete;

	/// Finds the first mode with a specific name.
	/** \returns nullptr when mode with this name is not found. */
	const Mode * findMode( const std::string & name ) const noexcept
	{
		for (const Mode & mode : modes)
			if (mode.name == name)
				return &mode;
		return nullptr;
	}

	/// Finds the first zone with a specific name.
	/** \returns nullptr when zone with this name is not found. */
	const Zone * findZone( const std::string & name ) const noexcept
	{
		for (const Zone & zone : zones)
			if (zone.name == name)
				return &zone;
		return nullptr;
	}

	/// Finds the first LED with a specific name.
	/** \returns nullptr when LED with this name is not found. */
	const LED * findLED( const std::string & name ) const noexcept
	{
		for (const LED & led : leds)
			if (led.name == name)
				return &led;
		return nullptr;
	}

#ifndef NO_EXCEPTIONS

	/// Exception-throwing variant of findModeX( const std::string & ) const.
	/** \throws NotFound when mode with this name is not found. */
	const Mode & findModeX( const std::string & name ) const
	{
		for (const Mode & mode : modes)
			if (mode.name == name)
				return mode;
		throw NotFound( "Mode of such name was not found" );
	}

	/// Exception-throwing variant of findZoneX( const std::string & ) const.
	/** \throws NotFound when zone with this name is not found. */
	const Zone & findZoneX( const std::string & name ) const
	{
		for (const Zone & zone : zones)
			if (zone.name == name)
				return zone;
		throw NotFound( "Zone of such name was not found" );
	}

	/// Exception-throwing variant of findLEDX( const std::string & ) const.
	/** \throws NotFound when LED with this name is not found. */
	const LED & findLEDX( const std::string & name ) const
	{
		for (const LED & led : leds)
			if (led.name == name)
				return led;
		throw NotFound( "LED of such name was not found" );
	}

#endif // NO_EXCEPTIONS

};


//======================================================================================================================
/** Convenience wrapper around iterator to container of pointers
  * that skips the additional needed dereference and returns a reference directly. */

template< typename IterType >
class PointerIterator
{
	IterType _origIter;
 public:
	PointerIterator( const IterType & origIter ) : _origIter( origIter ) {}
	auto operator*() -> decltype( **_origIter ) const { return **_origIter; }
	auto operator->() -> decltype( *_origIter ) const { return *_origIter; }
	PointerIterator & operator++() { ++_origIter; return *this; }
	PointerIterator operator++(int) { auto tmp = *this; ++_origIter; return tmp; }
	friend bool operator==(const PointerIterator & a, const PointerIterator & b) { return a._origIter == b._origIter; }
	friend bool operator!=(const PointerIterator & a, const PointerIterator & b) { return a._origIter != b._origIter; }
};


//======================================================================================================================
/// Searchable list of all RGB-capable devices detected by OpenRGB.

class DeviceList
{
	// This has to ve vector of pointers. Simple vector would invalidate references in Mode, Zone and LED when resized.
	using DeviceListType = std::vector< std::unique_ptr< Device > >;
	DeviceListType _list;

 public:

	DeviceList() noexcept {}
	// Copying is not allowed because it would break the non-owning references in Device sub-objects.
	DeviceList( const DeviceList & other ) = delete;
	DeviceList( DeviceList && other ) noexcept = default;
	DeviceList & operator=( const DeviceList & other ) = delete;
	DeviceList & operator=( DeviceList && other ) noexcept = default;

	size_t size() const noexcept { return _list.size(); }

	void append( DeviceDescription && desc ) { _list.emplace_back( new Device( nextIdx(), std::move( desc ) ) ); }
	void clear() noexcept { _list.clear(); }

	PointerIterator< DeviceListType::const_iterator > begin() const noexcept  { return _list.begin(); }
	PointerIterator< DeviceListType::const_iterator > end() const noexcept    { return _list.end(); }

	const Device & operator[]( uint32_t deviceIdx ) const noexcept { return *_list[ deviceIdx ]; }

	/// Iterate over all devices of specific type.
	template< typename FuncType >
	void forEach( DeviceType deviceType, FuncType loopBody ) const
	{
		for (const Device & device : *this)
			if (device.type == deviceType)
				loopBody( device );
	}

	/// Iterate over all devices of specific vendor.
	template< typename FuncType >
	void forEach( const std::string & vendor, FuncType loopBody ) const
	{
		for (const Device & device : *this)
			if (device.vendor == vendor)
				loopBody( device );
	}

	/// Finds the first device of specific type.
	/** \returns nullptr when device of this type is not found */
	const Device * find( DeviceType deviceType ) const noexcept
	{
		for (const Device & device : *this)
			if (device.type == deviceType)
				return &device;
		return nullptr;
	}

	/// Finds the first device with a specific name.
	/** \returns nullptr when device with this name is not found */
	const Device * find( const std::string & deviceName ) const noexcept
	{
		for (const Device & device : *this)
			if (device.name == deviceName)
				return &device;
		return nullptr;
	}

#ifndef NO_EXCEPTIONS

	/// Exception-throwing variant of find( DeviceType ) const
	/** \throws NotFound when device of this type is not found */
	const Device & findX( DeviceType deviceType ) const
	{
		for (const Device & device : *this)
			if (device.type == deviceType)
				return device;
		throw NotFound( "Device of such type was not found" );
	}

	/// Exception-throwing variant of find( const std::string & ) const.
	/** \throws NotFound when device with this name is not found */
	const Device & findX( const std::string & deviceName ) const
	{
		for (const Device & device : *this)
			if (device.name == deviceName)
				return device;
		throw NotFound( "Device of such name was not found" );
	}

#endif // NO_EXCEPTIONS

 private:

	uint32_t nextIdx() const noexcept { return uint32_t( _list.size() ); }

};


//======================================================================================================================
//  printing utils

void print( const Device & device, unsigned int indentLevel = 0 );
void print( const Mode & mode, unsigned int indentLevel = 0 );
void print( const Zone & zone, unsigned int indentLevel = 0 );
void print( const LED & led, unsigned int indentLevel = 0 );

void print( std::ostream & os, const Device & device, unsigned int indentLevel = 0 );
void print( std::ostream & os, const Mode & mode, unsigned int indentLevel = 0 );
void print( std::ostream & os, const Zone & zone, unsigned int indentLevel = 0 );
void print( std::ostream & os, const LED & led, unsigned int indentLevel = 0 );


//======================================================================================================================


} // namespace orgb


#endif // OPENRGB_DEVICE_INCLUDED
