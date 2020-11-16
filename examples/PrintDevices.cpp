//======================================================================================================================
//  lists all devices and their properties in console
//======================================================================================================================

#include <cstdio>

#include "OpenRGB/Client.hpp"
using orgb::ConnectStatus;
using orgb::DeviceListResult;
using orgb::RequestStatus;


int main( int /*argc*/, char * /*argv*/ [] )
{
	orgb::Client client( "My OpenRGB Client" );

	ConnectStatus status = client.connect( "127.0.0.1" );
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

	printf( "devices = [\n" );
	for (const orgb::Device & device : result.devices)
	{
		print( device, 1 );
	}
	printf( "]\n" );

	printf( "press enter to exit\n" );
	while (getchar() != '\n') {}

	return 0;
}
