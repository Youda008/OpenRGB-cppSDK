//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Created on:  1.11.2020
// Description: data structures containing device information
//======================================================================================================================

#ifndef OPENRGB_DEVICE_INCLUDED
#define OPENRGB_DEVICE_INCLUDED


#include "OpenRGB/private/Protocol.hpp"  // enums and flags

#include "Color.hpp"

#include <string>
#include <vector>
#include <list>


namespace orgb {


class Device;

struct LEDDescription;
struct ZoneDescription;
struct DeviceDescription;


//======================================================================================================================
/** Represents a particular LED on an RGB device. */

class LED
{

 public:

	const Device & parent;  ///< Warning: This is non-owning reference!! It will cease to be valid when the DeviceList is destructed

	uint32_t      id;
	std::string   name;
	uint32_t      value;  ///< device-specific value

 public:

	LED( const Device & parent, uint32_t id, LEDDescription && desc );

	// must be copyable so that vector can reallocate
	LED( const LED & other ) = default;
	LED( LED && other ) noexcept = default;

	LED & operator=( const LED & other ) = delete;
	LED & operator=( LED && other ) = delete;

};


//======================================================================================================================
/** Represents a group of LEDs on an RGB device. Only some devices have zones. */

class Zone
{

 public:

	const Device & parent;  ///< Warning: This is non-owning reference!! It will cease to be valid when the DeviceList is destructed

	uint32_t      id;
	std::string   name;
	ZoneType      type;
	uint32_t      minLeds;      ///< minimum size of the zone
	uint32_t      maxLeds;      ///< maximum size of the zone
	uint32_t      numLeds;      ///< current size of the zone
	uint32_t      matrixHeight; ///< if the zone type is matrix, this is its height
	uint32_t      matrixWidth;  ///< if the zone type is matrix, this is its width
	// TODO: the matrix values

 public:

	Zone( const Device & parent, uint32_t id, ZoneDescription && desc );

	// must be copyable so that vector can reallocate
	Zone( const Zone & other ) = default;
	Zone( Zone && other ) noexcept = default;

	Zone & operator=( const Zone & other ) = delete;
	Zone & operator=( Zone && other ) = delete;

};


//======================================================================================================================
/** Represents a color mode of an RGB device, like "breathing", "flashing", "rainbow" or "direct". */

class Mode
{

 public:

	const Device & parent;  ///< Warning: This is non-owning reference!! It will cease to be valid when the DeviceList is destructed

	const uint32_t      id;
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

	Mode( const Device & parent, uint32_t id, ModeDescription && desc );

	// must be copyable so that vector can reallocate
	Mode( const Mode & other ) = default;

	Mode & operator=( const Mode & other ) = delete;
	Mode & operator=( Mode && other ) = delete;

	void toProtocolDescription( ModeDescription & desc );

};


//======================================================================================================================
/** Represents an RGB-capable device. Device can have modes, zones and individual LEDs. */

class Device
{

 public:

	uint32_t id;
	DeviceType type;
	std::string name;
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

	Device( uint32_t id, DeviceDescription && descr );

	// no copies or moves allowed, because they would break the non-owning references in Modes, Zones and LEDs
	Device( const Device & other ) = delete;
	Device( Device && other ) noexcept = delete;
	Device & operator=( const Device & other ) = delete;
	Device & operator=( Device && other ) = delete;

	const Mode * findMode( const std::string & name ) const
	{
		for (const Mode & mode : modes)
			if (mode.name == name)
				return &mode;
		return nullptr;
	}

	const Zone * findZone( const std::string & name ) const
	{
		for (const Zone & zone : zones)
			if (zone.name == name)
				return &zone;
		return nullptr;
	}

	const LED * findLED( const std::string & name ) const
	{
		for (const LED & led : leds)
			if (led.name == name)
				return &led;
		return nullptr;
	}

};


//======================================================================================================================
/** All RGB-capable devices in your computer. */

class DeviceList
{
	// this has to be list, because vector would invalidate all references when resized
	using DeviceListType = std::list< Device >;
	DeviceListType _list;

 public:

	DeviceList() {}
	// copying is not allowed because it would break the non-owning references in Device sub-objects
	DeviceList( const DeviceList & other ) = delete;
	DeviceList( DeviceList && other ) noexcept = default;
	DeviceList & operator=( const DeviceList & other ) = delete;
	DeviceList & operator=( DeviceList && other ) noexcept = default;

	void append( uint32_t deviceId, DeviceDescription && desc ) { _list.emplace_back( deviceId, std::move( desc ) ); }
	void clear() { _list.clear(); }

	DeviceListType::const_iterator begin() const  { return _list.begin(); }
	DeviceListType::const_iterator end() const    { return _list.end(); }

	template< typename FuncType >
	void forEach( DeviceType deviceType, FuncType loopBody ) const
	{
		for (const Device & device : _list)
			if (device.type == deviceType)
				loopBody( device );
	}

	template< typename FuncType >
	void forEach( const std::string & deviceName, FuncType loopBody ) const
	{
		for (const Device & device : _list)
			if (device.name == deviceName)
				loopBody( device );
	}

	const Device * find( DeviceType deviceType ) const
	{
		for (const Device & device : _list)
			if (device.type == deviceType)
				return &device;
		return nullptr;
	}

	const Device * find( const std::string & deviceName ) const
	{
		for (const Device & device : _list)
			if (device.name == deviceName)
				return &device;
		return nullptr;
	}

};


//======================================================================================================================
//  printing utils

// TODO: output to abstract C++ stream

void print( const Device & device, unsigned int indentLevel = 0 );
void print( const Mode & mode, unsigned int indentLevel = 0 );
void print( const Zone & zone, unsigned int indentLevel = 0 );
void print( const LED & led, unsigned int indentLevel = 0 );


//======================================================================================================================


} // namespace orgb


#endif // OPENRGB_DEVICE_INCLUDED
