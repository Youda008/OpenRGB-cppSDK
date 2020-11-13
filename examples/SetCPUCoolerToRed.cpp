//======================================================================================================================
//  sets all leds of a CPU cooler to red color
//======================================================================================================================

#include <cstdio>

#include "OpenRGB/Client.hpp"
using orgb::ConnectStatus;
using orgb::DeviceListResult;
using orgb::RequestStatus;
using orgb::DeviceType;
using orgb::Device;
using orgb::Color;


int main( int /*argc*/, char * /*argv*/ [] )
{
	orgb::Client client( "My OpenRGB Client" );

	ConnectStatus status = client.connect( "192.168.0.5" );
	if (status != ConnectStatus::SUCCESS)
	{
		fprintf( stderr, "failed to connect\n" );
		return 1;
	}

	DeviceListResult result = client.requestDeviceList();
	if (result.status != RequestStatus::SUCCESS)
	{
		fprintf( stderr, "failed to get device list\n" );
		return 2;
	}

	const Device * cpuCooler = result.devices.find( DeviceType::COOLER );
	if (!cpuCooler)
	{
		fprintf( stderr, "device CPU cooler not found.\n" );
		return 3;
	}

	client.setDeviceColor( *cpuCooler, Color::RED );

	return 0;
}
