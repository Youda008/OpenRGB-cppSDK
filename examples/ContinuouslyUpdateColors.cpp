//======================================================================================================================
//  entrypoint for the library testing and debuging
//======================================================================================================================

#include <cstdio>    // printf
#include <cstdlib>   // signal
#include <thread>    // sleep
using namespace std::chrono;

#include "OpenRGB/Client.hpp"
using orgb::ConnectStatus;
using orgb::RequestStatus;
using orgb::UpdateStatus;
using orgb::DeviceListResult;
using orgb::DeviceList;
using orgb::DeviceType;
using orgb::Color;


static bool keepRunning = false;

void signalFunc( int )
{
	keepRunning = false;
	printf( "INTERRUPT signal received, quitting...\n" );
}

int main( int /*argc*/, char * /*argv*/ [] )
{
	static const char * hostName = "192.168.0.106";  // you can also use the NetBIOS computer name

	orgb::Client client( "My OpenRGB Client" );

	// a clean way to quit the application without killing it by force
	keepRunning = true;
	signal( SIGINT, signalFunc );

	DeviceList devices;

	const Color colors [] = {
		{ 255,   0,   0 },
		{ 255, 255,   0 },
		{   0, 255,   0 },
		{   0, 255, 255 },
		{   0,   0, 255 },
		{ 255,   0, 255 },
	};
	unsigned int currentColorIdx = 0;

	while (keepRunning)
	{
		// reconnect if the connection was lost
		if (!client.isConnected())
		{
			printf( "trying to connect to %s\n", hostName );
			ConnectStatus status = client.connect( hostName );
			if (status == ConnectStatus::HOST_NOT_RESOLVED)
			{
				printf( "host %s not found\n", hostName );
				break;
			}
			else if (status != ConnectStatus::SUCCESS)
			{
				printf( "connection failed\n" );
				continue;
			}
		}

		// update our local device list, if it has changed on the server
		UpdateStatus updateStatus = client.checkForDeviceUpdates();
		if (updateStatus == UpdateStatus::OUT_OF_DATE)
		{
			printf( "updating device list\n" );
			DeviceListResult result = client.requestDeviceList();
			if (result.status != RequestStatus::SUCCESS)
			{
				printf( "failed to get device list\n" );
				// reset everything and try again
				client.disconnect();
				continue;
			}
			devices = std::move( result.devices );
		}
		else if (updateStatus != UpdateStatus::UP_TO_DATE)
		{
			// some error occured while trying to find if the device list is up to date
			// reset everything and try again
			printf( "socket is broken, reseting connection\n" );
			client.disconnect();
			continue;
		}

		// connected and updated, let's change our colors
		currentColorIdx = (currentColorIdx + 1) % (sizeof(colors) / sizeof(Color));
		Color nextColor = colors[ currentColorIdx ];

		for (const orgb::Device & device : devices)
		{
			if (device.type == DeviceType::COOLER)
			{
				printf( "setting CPU cooler to " ); print( nextColor ); putchar('\n');
				client.setDeviceColor( device, nextColor );
			}
		}

		std::this_thread::sleep_for( milliseconds( 1000 ) );
	}

	return 0;
}
