//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: OpenRGB network client
//======================================================================================================================

#ifndef OPENRGB_CLIENT_INCLUDED
#define OPENRGB_CLIENT_INCLUDED


#include "DeviceInfo.hpp"
#include "Color.hpp"
#include "NetAddress.hpp"
#include "SystemErrorType.hpp"  // HACK: read the comment at the top of that header file

#include <string>  // client name
#include <memory>  // unique_ptr<Socket>
#include <chrono>  // timeout

namespace own {
	class TcpSocket;
}


namespace orgb {


constexpr uint16_t defaultPort = 6742;


//======================================================================================================================

/// All the possible ways how the connect operation can end up
enum class ConnectStatus
{
	Success,                 ///< The operation was successful.
	NetworkingInitFailed,    ///< Operation failed because underlying networking system could not be initialized. Call getLastSystemError() for more info.
	AlreadyConnected,        ///< Connect operation failed because the socket is already connected. Call disconnect() first.
	HostNotResolved,         ///< The hostname you entered could not be resolved to IP address. Call getLastSystemError() for more info.
	ConnectFailed,           ///< Could not connect to the target server, either it's down or the port is closed. Call getLastSystemError() for more info.
	RequestVersionFailed,    ///< Failed to send the client's protocol version or receive the server's protocol version. Call getLastSystemError() for more info.
	VersionNotSupported,     ///< The protocol version of the server is not supported. Please update the OpenRGB app.
	SendNameFailed,          ///< Failed to send the client name to the server. Call getLastSystemError() for more info.
	OtherSystemError,        ///< Other system error. Call getLastSystemError() for more info.
	UnexpectedError,         ///< Internal error of this library. This should not happen unless there is a mistake in the code, please create a github issue.
};
const char * enumString( ConnectStatus status ) noexcept;

/// All the possible ways how a request can end up
enum class RequestStatus
{
	Success,            ///< The request was succesful.
	NotConnected,       ///< Request failed because the client is not connected. Call connect() first.
	SendRequestFailed,  ///< Failed to send the request message.
	ConnectionClosed,   ///< Server has closed the connection.
	NoReply,            ///< No reply has arrived from the server in given timeout. In case this happens too often, you may try to increase the timeout.
	ReceiveError,       ///< There has been some other error while trying to receive a reply. Call getLastSystemError() for more info.
	InvalidReply,       ///< The reply from the server is invalid.
	UnexpectedError,    ///< Internal error of this library. This should not happen unless there is a mistake in the code, please create a github issue.
};
const char * enumString( RequestStatus status ) noexcept;

/// All the possible results of a check whether the locally stored device list is out of date
enum class UpdateStatus
{
	UpToDate,           ///< The current device list seems up to date.
	OutOfDate,          ///< Server has sent a notification message indicating that the device list has changed. Call requestDeviceList() again.
	ConnectionClosed,   ///< Server has closed the connection.
	UnexpectedMessage,  ///< Server has sent some other kind of message that we didn't expect.
	CantRestoreSocket,  ///< Error has occured while trying to restore socket to its original state and the socket has been closed. Call getLastSystemError() for more info. This should never happen, but one never knows.
	OtherSystemError,   ///< Other system error. Call getLastSystemError() for more info.
	UnexpectedError,    ///< Internal error of this library. This should not happen unless there is a mistake in the code, please create a github issue.
};
const char * enumString( UpdateStatus status ) noexcept;

/// Result and output of a device list request
struct DeviceListResult
{
	RequestStatus status;
	DeviceList devices;
};


//======================================================================================================================
/// OpenRGB network client.
/** Use this to communicate with the OpenRGB service in order to set colors on your RGB devices. */

class Client
{

 public:

	/// Creates a client of specified or default name. Does not connect anywhere yet.
	Client( const std::string & clientName = "orgb::Client" ) noexcept;

	~Client() noexcept;

	// The connection cannot be shared.
	Client( const Client & other ) = delete;

	Client( Client && other ) noexcept = default;
	Client & operator=( Client && other ) noexcept = default;

	/// Connects to the OpenRGB server determined by a host name and announces our client name.
	ConnectStatus connect( const std::string & host, uint16_t port = defaultPort ) noexcept;

	/// Connects to the OpenRGB server determined by IP address and announces our client name.
	ConnectStatus connect( own::IPAddr addr = {127,0,0,1}, uint16_t port = defaultPort ) noexcept;

	/// Closes connection to the server.
	void disconnect() noexcept;

	/// Tells whether the client is currently connected to a server.
	bool isConnected() const noexcept;

	/// Sets a timeout for receiving request answers.
	bool setTimeout( std::chrono::milliseconds timeout ) noexcept;

	/// Queries the server for information about all its RGB devices.
	DeviceListResult requestDeviceList() noexcept;

	/// Checks if the device list you downloaded earlier via requestDeviceList() hasn't been changed on the server.
	/** In case it has been changed, you need to call requestDeviceList() again. */
	UpdateStatus checkForDeviceUpdates() noexcept;

	/// Updates the parameters of a mode and also switches the device to this mode.
	/** If you just want to switch the mode, use one of the Mode objects received from the server via requestDeviceList().
	  * If you want to change the parameters of a mode, create a copy of the Mode object, change the parameters of the copy
	  * and pass the copy to this function. */
	RequestStatus changeMode( const Device & device, const Mode & mode ) noexcept;

	/// Switches the device to a directly controlled color mode.
	/** This seems unsupported by many RGB controllers, and it's probably deprecated in the OpenRGB app. */
	RequestStatus switchToCustomMode( const Device & device ) noexcept;

	/// Sets one unified color for the whole device.
	RequestStatus setDeviceColor( const Device & device, Color color ) noexcept;

	/// Sets a color of a particular zone of a device.
	RequestStatus setZoneColor( const Zone & zone, Color color ) noexcept;

	/// Resizes a zone of leds, if the device supports it.
	RequestStatus setZoneSize( const Zone & zone, uint32_t newSize ) noexcept;

	/// Sets a color of one selected LED.
	RequestStatus setColorOfSingleLED( const LED & led, Color color ) noexcept;

#ifndef NO_EXCEPTIONS

	/// Exception-throwing variant of connect( const std::string &, uint16_t ).
	/** \throws UserError
	  * \throws ConnectionError
	  * \throws SystemError */
	void connectX( const std::string & host, uint16_t port = 6742 );

	/// Exception-throwing variant of connect( own::IPAddr addr, uint16_t port ).
	/** \throws UserError
	  * \throws ConnectionError
	  * \throws SystemError */
	void connectX( own::IPAddr addr = {127,0,0,1}, uint16_t port = 6742 );

	/// Exception-throwing variant of setTimeout( std::chrono::milliseconds ).
	/** \throws SystemError */
	void setTimeoutX( std::chrono::milliseconds timeout );

	/// Exception-throwing variant of requestDeviceList().
	/** \throws UserError
	  * \throws ConnectionError
	  * \throws SystemError */
	DeviceList requestDeviceListX();

	/// Exception-throwing variant of checkForDeviceUpdates().
	/** \throws ConnectionError
	  * \throws SystemError */
	bool isDeviceListOutdatedX();

	/// Exception-throwing variant of changeMode( const Device &, const Mode & ).
	/** \throws UserError
	  * \throws ConnectionError
	  * \throws SystemError */
	void changeModeX( const Device & device, const Mode & mode );

	/// Exception-throwing variant of switchToDirectMode( const Device & ).
	/** \throws UserError
	  * \throws ConnectionError
	  * \throws SystemError */
	void switchToDirectModeX( const Device & device );

	/// Exception-throwing variant of setDeviceColor().
	/** \throws UserError
	  * \throws ConnectionError
	  * \throws SystemError */
	void setDeviceColorX( const Device & device, Color color );

	/// Exception-throwing variant of setZoneColor( const Zone &, Color ).
	/** \throws UserError
	  * \throws ConnectionError
	  * \throws SystemError */
	void setZoneColorX( const Zone & zone, Color color );

	/// Exception-throwing variant of setZoneSize( const Zone &, uint32_t ).
	/** \throws UserError
	  * \throws ConnectionError
	  * \throws SystemError */
	void setZoneSizeX( const Zone & zone, uint32_t newSize );

	/// Exception-throwing variant of setColorOfSingleLED( const LED &, Color ).
	/** \throws UserError
	  * \throws ConnectionError
	  * \throws SystemError */
	void setColorOfSingleLEDX( const LED & led, Color color );

#endif // NO_EXCEPTIONS

	/// Returns the system error code that caused the last failure.
	system_error_t getLastSystemError() const noexcept;

	/// Converts the numeric value of the last system error to a user-friendly string.
	std::string getLastSystemErrorStr() const noexcept;

	/// Converts the given numeric error code to a user-friendly string.
	std::string getSystemErrorStr( system_error_t errorCode ) const noexcept;

 private: // helpers

	template< typename HostSpec >
	ConnectStatus _connect( const HostSpec & host, uint16_t port );
	DeviceListResult _requestDeviceList();
	RequestStatus _changeMode( const Device & device, const Mode & mode );
	RequestStatus _switchToCustomMode( const Device & device );
	RequestStatus _setDeviceColor( const Device & device, Color color );
	RequestStatus _setZoneColor( const Zone & zone, Color color );
	RequestStatus _setZoneSize( const Zone & zone, uint32_t newSize );
	RequestStatus _setColorOfSingleLED( const LED & led, Color color );

	template< typename Message, typename ... ConstructorArgs >
	bool sendMessage( ConstructorArgs ... args );

	template< typename Message >
	struct RecvResult
	{
		RequestStatus status;
		Message message;
	};
	template< typename Message >
	RecvResult< Message > awaitMessage() noexcept;

	UpdateStatus checkForUpdateMessageArrival() noexcept;

#ifndef NO_EXCEPTIONS
	void connectStatusToException( ConnectStatus status );
	void requestStatusToException( RequestStatus status );
#endif // NO_EXCEPTIONS

 private:

	std::string _clientName;

	// a pointer so that we don't have to include the TcpSocket and all its OS dependancies here
	std::unique_ptr< own::TcpSocket > _socket;

	bool _isDeviceListOutOfDate;

};


//======================================================================================================================


} // namespace orgb


#endif // OPENRGB_CLIENT_INCLUDED
