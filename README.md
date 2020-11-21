# OpenRGB-cppSDK

A C++ library allowing you to control OpenRGB (https://gitlab.com/CalcProgrammer1/OpenRGB) via network.
Designed for minimal CPU and memory overhead, does not create any threads in the background, gives you full control your application main loop.


## Usage

Include the library declarations.
(optionally declare what elements you will be using, for shorter identifiers)
```
#include "OpenRGB/Client.hpp"
using orgb::ConnectStatus;
using orgb::DeviceListResult;
using orgb::RequestStatus;
using orgb::DeviceType;
```

Create the client and connect to OpenRGB server.
```
orgb::Client client( "My OpenRGB Client" );

ConnectStatus status = client.connect( "127.0.0.1" );  // you can also use Windows computer name
if (status != ConnectStatus::Success)
{
	fprintf( stderr, "failed to connect (error code: %d)\n", int( client.getLastSystemError() ) );
	return;
}
```

Download a list of devices and their settings.
```
DeviceListResult result = client.requestDeviceList();
if (result.status != RequestStatus::Success)
{
	fprintf( stderr, "failed to get device list\n" );
	return;
}
```

Find the device you want to control. You can search by type or name.
```
const Device * cpuCooler = result.devices.find( DeviceType::Cooler );
if (!cpuCooler)
{
	fprintf( stderr, "device CPU cooler not found.\n" );
	return;
}
```

Some devices don't accept manual color changes until you set them to "Direct" mode.
This is how you do it.
```
const Mode * directMode = cpuCooler->findMode( "Direct" );
if (!directMode)
{
	fprintf( stderr, "\"Direct\" mode not found in CPU cooler.\n" );
	return;
}
client.changeMode( *cpuCooler, *directMode );
```

Finally, set any color you want.
```
client.setDeviceColor( *cpuCooler, Color::RED );
```

You can create any color by using the `Color` constructor.
```
Color customColor( 255, 128, 64 );
```

#### !!WARNING!!
Between any color or mode change requests there should be at least few millisecond delay. Current implementation of OpenRGB is unreliable and bugs itself when you send it multiple requests at once.

Add this between any two calls to client.
```
std::this_thread::sleep_for( milliseconds( 50 ) );
```

#### Exceptions vs return values
If you don't like the old-school way of checking return values, there are exception-throwing variants for each method of the client (they are postfixed with `X`). The code can be then written in slightly simplier way.
```
try
{
	orgb::Client client( "My OpenRGB Client" );

	client.connectX( "127.0.0.1" );

	DeviceList devices = client.requestDeviceListX();

	const Device & cpuCooler = devices.findX( DeviceType::Cooler );

	const Mode & directMode = cpuCooler->findModeX( "Direct" );

	client.changeModeX( cpuCooler, directMode );

	client.setDeviceColorX( cpuCooler, Color::RED );
}
catch (const orgb::Exception & ex)
{
	printf( "Error: %s\n", ex.errorMessage() );
}
```

#### More examples
Complete examples of working apps using this library can be found in directory `examples`, feel free to copy&paste anything.

#### Building your application
Depending on your IDE or build system, you must add the directory `include` to your include directories and the directory where you built this library to your link library directories. Then you must link library `orgbsdk` to your app. The library is static, so you don't have to worry about moving any dynamic librariess around together with your app.

Simpliest example with GCC
```
g++ -I"<path to OpenRGB-cppSDK>/include" -L"<path to OpenRGB-cppSDK>/build" -o myapp.exe myapp.cpp -lorgbsdk
```


## How to build the library

### Windows - Msys2

```
mkdir build-release
cd build-release
cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ../
mingw32-make
```

### Windows - Visual Studio

```
mkdir build-release
cd build-release
cmake -G "Visual Studio 16 2019" -DCMAKE_BUILD_TYPE=Release ../
```
Then open the resulting solution in Visual Studio and build it.

### Linux
```
mkdir build-release
cd build-release
cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release ../
make
```
