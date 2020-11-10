//======================================================================================================================
//  sets all leds of a CPU cooler to red color
//======================================================================================================================

#include <cstdio>

#include "OpenRGB/Client.hpp"
using orgb::ConnectStatus;
using orgb::DeviceListResult;
using orgb::RequestStatus;
using orgb::DeviceType;
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

	for (const orgb::Device & device : result.devices)
	{
		if (device.type == DeviceType::COOLER)
		{
			client.setDeviceColor( device, Color::RED );
		}
	}

	return 0;
}
