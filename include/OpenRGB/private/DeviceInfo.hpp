//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Created on:  1.11.2020
// Description: data structures containing device information
//======================================================================================================================

#ifndef OPENRGB_DEVICE_INCLUDED
#define OPENRGB_DEVICE_INCLUDED


#include "OpenRGB/private/Protocol.hpp"

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
/** TODO */

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
/** TODO */

class Zone
{

 public:

	const Device & parent;  ///< Warning: This is non-owning reference!! It will cease to be valid when the DeviceList is destructed

	uint32_t      id;
	std::string   name;
	ZoneType      type;
	uint32_t      minLeds;
	uint32_t      maxLeds;
	uint32_t      numLeds;
	uint32_t      matrixHeight;
	uint32_t      matrixWidth;

 public:

	Zone( const Device & parent, uint32_t id, ZoneDescription && desc );

	// must be copyable so that vector can reallocate
	Zone( const Zone & other ) = default;
	Zone( Zone && other ) noexcept = default;

	Zone & operator=( const Zone & other ) = delete;
	Zone & operator=( Zone && other ) = delete;

};


//======================================================================================================================
/** TODO */

class Mode
{

 public:

	const Device & parent;  ///< Warning: This is non-owning reference!! It will cease to be valid when the DeviceList is destructed

	uint32_t      id;
	std::string   name;
	uint32_t      value;  ///< device-specific value
	uint32_t      flags;  ///< see ModeFlags for possible bit flags
	Direction     direction;
	uint32_t      minSpeed;
	uint32_t      maxSpeed;
	uint32_t      speed;
	uint32_t      minColors;
	uint32_t      maxColors;
	ColorMode     colorMode;
	std::vector< Color > colors;

 public:

	Mode( const Device & parent, uint32_t id, ModeDescription && desc );

	// must be copyable so that vector can reallocate
	Mode( const Mode & other ) = default;
	Mode( Mode && other ) noexcept = default;

	Mode & operator=( const Mode & other ) = delete;
	Mode & operator=( Mode && other ) = delete;

	void toProtocolDescription( ModeDescription & desc );

};


//======================================================================================================================
/** TODO */

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

};


//======================================================================================================================
/** TODO */

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

	DeviceListType::const_iterator begin() const  { return _list.begin(); }
	DeviceListType::const_iterator end() const    { return _list.end(); }

};


//======================================================================================================================
//  printing utils

// TODO: output to abstract stream

void print( const Device & device, unsigned int indentLevel = 0 );
void print( const Mode & mode, unsigned int indentLevel = 0 );
void print( const Zone & zone, unsigned int indentLevel = 0 );
void print( const LED & led, unsigned int indentLevel = 0 );


//======================================================================================================================


} // namespace orgb


#endif // OPENRGB_DEVICE_INCLUDED
