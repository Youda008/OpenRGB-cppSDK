#include "Commands.hpp"

#include "Essential.hpp"
#include "LangUtils.hpp"
#include "StreamUtils.hpp"

#include "OpenRGB/Client.hpp"
using namespace orgb;

#include "CommandRegistration.hpp"

#include <string>
#include <vector>
using namespace std;


//======================================================================================================================
//  auto-registration

RegisteredCommands g_standardCommands;  ///< commands equally usable in all modes

RegisteredCommands g_specialCommands;  ///< special type of commands that are used differently in different modes

/// Convenience macro to add and automatically register a stadard command.
/** When used, this command will then be searchable in g_standardCommands by its name. */
#define REGISTER_COMMAND( NAME, ARG_NAMES, DESC, HANDLER_FUNC ) \
	static const RegisteredCommand NAME##Cmd( #NAME, ARG_NAMES, DESC, HANDLER_FUNC ); \
	static const char autoreg_##NAME = [](){ g_standardCommands.registerCommand( NAME##Cmd ); return 'R'; }();

/// Convenience macro to add and automatically register a special command.
/** This will export the symbol of the command object, so that it can be used directly without map lookup. */
#define REGISTER_SPECIAL_COMMAND( NAME, ARG_NAMES, DESC, HANDLER_FUNC ) \
	const RegisteredCommand NAME##Cmd( #NAME, ARG_NAMES, DESC, HANDLER_FUNC ); \
	static const char autoreg_##NAME = [](){ g_standardCommands.registerCommand( NAME##Cmd ); return 'S'; }();

/// Workaround for macros not recognizing brace initializers with commas
/** https://stackoverflow.com/questions/29578902/why-cant-you-use-c11-brace-initialization-with-macros */
#define HANDLER( ... ) []( MAYBE_UNUSED orgb::Client & client, MAYBE_UNUSED const ArgList & args ) __VA_ARGS__
#define ARGS( ... ) { __VA_ARGS__ }


//======================================================================================================================
//  compound arguments used by the commands

struct Endpoint
{
	std::string hostName;
	uint16_t port;
};
istream & operator>>( istream & is, Endpoint & endpoint )
{
	own::read_until( is, endpoint.hostName, ':' );  // TODO: until : or space
	if (!is.good())  // ':' not found
	{
		endpoint.port = 0;
		return is;
	}

	is >> endpoint.port;

	return is;
}

struct PartID
{
	// usecase for std::variant but that is C++17 and we are only C++11
	std::string str;
	uint32_t idx;
};
istream & operator>>( istream & is, PartID & partID )
{
	is >> partID.str;
	// int representation is optional, don't make the stream fail if it's not int
	if (sscanf( partID.str.c_str(), "%u", &partID.idx ) != 1)
		partID.idx = 0;
	return is;
}

struct PartSpec
{
	enum class Type { Zone, Led } type;
	PartID id;

	bool isEmpty() const { return id.str.empty(); }
};
istream & operator>>( istream & is, PartSpec & partSpec )
{
	std::string typeStr = own::read_until( is, ':' );  // TODO: until : or space
	if (!is.good())  // ':' was not found
	{
		is.setstate( std::ios::failbit );
		return is;
	}

	is >> partSpec.id;

	own::to_lower( typeStr );
	if (typeStr == "zone")
		partSpec.type = PartSpec::Type::Zone;
	else if (typeStr == "led")
		partSpec.type = PartSpec::Type::Led;
	else
		is.setstate( std::ios::failbit );

	return is;
}


//======================================================================================================================
//  helpers

const Device * findDevice( const DeviceList & devices, const PartID & deviceID )
{
	const Device * device = nullptr;
	if (deviceID.idx != 0)
	{
		if (deviceID.idx >= devices.size())
			cout << "Device with index " << deviceID.idx << " does not exist." << endl;
		else
			device = &devices[ deviceID.idx ];
	}
	else
	{
		device = devices.find( deviceID.str );
		if (!device)
			cout << "Device with name " << deviceID.str << " not found." << endl;
		// TODO: maybe according to vendor?
	}
	return device;
}

const Zone * findZone( const Device & device, const PartID & zoneID )
{
	const Zone * zone = nullptr;
	if (zoneID.idx != 0)
	{
		if (zoneID.idx >= device.zones.size())
			cout << "Zone with index " << zoneID.idx << " does not exist." << endl;
		else
			zone = &device.zones[ zoneID.idx ];
	}
	else
	{
		zone = device.findZone( zoneID.str );
		if (!zone)
			cout << "Zone with name " << zoneID.str << " not found." << endl;
		// TODO: maybe according to vendor?
	}
	return zone;
}

const LED * findLED( const Device & device, const PartID & ledID )
{
	const LED * led = nullptr;
	if (ledID.idx != 0)
	{
		if (ledID.idx >= device.leds.size())
			cout << "LED with index " << ledID.idx << " does not exist." << endl;
		else
			led = &device.leds[ ledID.idx ];
	}
	else
	{
		led = device.findLED( ledID.str );
		if (!led)
			cout << "LED with name " << ledID.str << " not found." << endl;
		// TODO: maybe according to vendor?
	}
	return led;
}

const Mode * findMode( const Device & device, const PartID & modeID )
{
	const Mode * mode = nullptr;
	if (modeID.idx != 0)
	{
		if (modeID.idx >= device.modes.size())
			cout << "Mode with index " << modeID.idx << " does not exist." << endl;
		else
			mode = &device.modes[ modeID.idx ];
	}
	else
	{
		mode = device.findMode( modeID.str );
		if (!mode)
			cout << "Mode with name " << modeID.str << " not found." << endl;
		// TODO: maybe according to vendor?
	}
	return mode;
}


//======================================================================================================================
//  commands

REGISTER_SPECIAL_COMMAND( help, "", "prints this list of commands", HANDLER(
{
	cout << "Possible commands:\n";
	for (const RegisteredCommand * cmd : g_specialCommands)
	{
		cout << "  " << *cmd << '\n';
	}
	for (const RegisteredCommand * cmd : g_standardCommands)
	{
		cout << "  " << *cmd << '\n';
	}
	cout << endl;
	return true;
}))

REGISTER_SPECIAL_COMMAND( exit, "", "quits this application", HANDLER(
{
	// This is only formal, it will never be called, because special commands are handled separately in the main.
	exit(0);
	return true;
}))

REGISTER_SPECIAL_COMMAND( connect, "<host_name>[:<port>]", "connects to an OpenRGB server", HANDLER(
{
	Endpoint endpoint = args.getNext< Endpoint >();
	if (endpoint.port == 0)
		endpoint.port = orgb::defaultPort;

	cout << "Connecting to " << endpoint.hostName << ":" << endpoint.port << endl;
	ConnectStatus status = client.connect( endpoint.hostName, endpoint.port );

	if (status == ConnectStatus::Success)
	{
		cout << " -> success" << endl;
		return true;
	}
	else
	{
		cout << " -> failed: " << enumString( status ) << " (error code: " << client.getLastSystemError() << ")" << endl;
		return false;
	}
}))

REGISTER_SPECIAL_COMMAND( disconnect, "", "disconnects from the currently connected server", HANDLER(
{
	client.disconnect();
	cout << "Disconnected." << endl;
	return true;
}))

REGISTER_COMMAND( list, "", "lists all devices and their properties, modes, zones and LEDs", HANDLER(
{
	cout << "Requesting the device list." << endl;
	DeviceListResult result = client.requestDeviceList();

	if (result.status != RequestStatus::Success)
	{
		cout << " -> failed: " << enumString( result.status ) << endl;
		return false;
	}

	cout << '\n' << "devices = [" << '\n';
	for (const orgb::Device & device : result.devices)
	{
		print( cout, device, 1 );
	}
	cout << ']' << '\n' << endl;

	return true;
}))

REGISTER_COMMAND( setcolor, "<device_id> [(zone|led):<id>] <color>", "changes a color of the whole device or a particular zone or led", HANDLER(
{
	PartID deviceID = args.getNext< PartID >();
	PartSpec partSpec;
	if (args.size() >= 3)
	{
		partSpec = args.getNext< PartSpec >();
	}
	Color color = args.getNext< Color >();

	// Device list cannot be re-used from the previous 'list' command, because that command may have been executed in
	// a different process in non-interactive mode or not executed at all.
	DeviceListResult listResult = client.requestDeviceList();
	if (listResult.status != RequestStatus::Success)
	{
		cout << "Failed to get a recent device list: " << enumString( listResult.status ) << endl;
		return false;
	}

	const Device * device = findDevice( listResult.devices, deviceID );
	if (!device)
		return false;

	RequestStatus status;
	if (partSpec.isEmpty())
	{
		cout << "Changing color of device " << deviceID.str << " to " << color << endl;
		status = client.setDeviceColor( *device, color );
	}
	else if (partSpec.type == PartSpec::Type::Zone)
	{
		const Zone * zone = findZone( *device, partSpec.id );
		if (!zone)
			return false;
		cout << "Changing color of zone " << partSpec.id.str << " to " << color << endl;
		status = client.setZoneColor( *zone, color );
	}
	else
	{
		const LED * led = findLED( *device, partSpec.id );
		if (!led)
			return false;
		cout << "Changing color of LED " << partSpec.id.str << " to " << color << endl;
		status = client.setColorOfSingleLED( *led, color );
	}

	if (status == RequestStatus::Success)
	{
		cout << " -> success" << endl;
		return true;
	}
	else
	{
		cout << " -> failed: " << enumString( status ) << endl;
		return false;
	}
}))

REGISTER_COMMAND( setmode, "<device_id> <mode>", "no idea, ask the OpenRGB devs", HANDLER(
{
	PartID deviceID = args.getNext< PartID >();
	PartID modeID = args.getNext< PartID >();

	// Device list cannot be re-used from the previous 'list' command, because that command may have been executed in
	// a different process in non-interactive mode or not executed at all.
	DeviceListResult listResult = client.requestDeviceList();
	if (listResult.status != RequestStatus::Success)
	{
		cout << "Failed to get a recent device list: " << enumString( listResult.status ) << endl;
		return false;
	}

	const Device * device = findDevice( listResult.devices, deviceID );
	if (!device)
		return false;

	const Mode * mode = findMode( *device, modeID );
	if (!mode)
		return false;

	cout << "Changing mode of device " << deviceID.str << " to " << modeID.str << endl;
	RequestStatus status = client.changeMode( *device, *mode );

	if (status == RequestStatus::Success)
	{
		cout << " -> success" << endl;
		return true;
	}
	else
	{
		cout << " -> failed: " << enumString( status ) << endl;
		return false;
	}
}))

REGISTER_COMMAND( custommode, "<device_id>", "no idea, ask the OpenRGB devs", HANDLER(
{
	PartID deviceID = args.getNext< PartID >();

	// Device list cannot be re-used from the previous 'list' command, because that command may have been executed in
	// a different process in non-interactive mode or not executed at all.
	DeviceListResult listResult = client.requestDeviceList();
	if (listResult.status != RequestStatus::Success)
	{
		cout << "Failed to get a recent device list: " << enumString( listResult.status ) << endl;
		return false;
	}

	const Device * device = findDevice( listResult.devices, deviceID );
	if (!device)
		return false;

	cout << "Swithing device " << deviceID.str << " to custom mode" << endl;
	RequestStatus status = client.switchToCustomMode( *device );

	if (status == RequestStatus::Success)
	{
		cout << " -> success" << endl;
		return true;
	}
	else
	{
		cout << " -> failed: " << enumString( status ) << endl;
		return false;
	}
}))

REGISTER_COMMAND( resizezone, "<device_id> <zone_id> <size>", "resizes a selected zone of a device", HANDLER(
{
	PartID deviceID = args.getNext< PartID >();
	PartID zoneID = args.getNext< PartID >();
	uint32_t zoneSize = args.getNext< uint32_t >();

	// Device list cannot be re-used from the previous 'list' command, because that command may have been executed in
	// a different process in non-interactive mode or not executed at all.
	DeviceListResult listResult = client.requestDeviceList();
	if (listResult.status != RequestStatus::Success)
	{
		cout << "Failed to get a recent device list: " << enumString( listResult.status ) << endl;
		return false;
	}

	const Device * device = findDevice( listResult.devices, deviceID );
	if (!device)
		return false;

	const Zone * zone = findZone( *device, zoneID );
	if (!zone)
		return false;

	cout << "Changing size of zone " << zoneID.str << " to " << zoneSize << endl;
	RequestStatus status = client.setZoneSize( *zone, zoneSize );

	if (status == RequestStatus::Success)
	{
		cout << " -> success" << endl;
		return true;
	}
	else
	{
		cout << " -> failed: " << enumString( status ) << endl;
		return false;
	}
}))
