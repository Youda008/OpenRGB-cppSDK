//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Created on:  1.11.2020
// Description: OpenRGB network client
//======================================================================================================================

#ifndef OPENRGB_CLIENT_INCLUDED
#define OPENRGB_CLIENT_INCLUDED


#include "private/Color.hpp"
#include "private/DeviceInfo.hpp"
#include "private/SystemErrorInfo.hpp"  // error_code_t for getLastError return values

#include <string>  // client name
#include <memory>  // unique_ptr<Socket>


namespace orgb {


class TcpClientSocket;


//======================================================================================================================

/** TODO */
enum class ConnectStatus
{
	SUCCESS,                 ///< The operation was successful.
	NETWORKING_INIT_FAILED,  ///< Operation failed because underlying networking system could not be initialized. Call getLastSystemError() for more info.
	ALREADY_CONNECTED,       ///< Connect operation failed because the socket is already connected. Call disconnect() first.
	HOST_NOT_RESOLVED,       ///< The hostname you entered could not be resolved to IP address. Call getLastSystemError() for more info.
	CONNECT_FAILED,          ///< Could not connect to the target server, either it's down or the port is closed. Call getLastSystemError() for more info.
	SEND_NAME_FAILED,        ///< Failed to send the client name to the server. Call getLastSystemError() for more info.
	OTHER_ERROR              ///< Other system error. Call getLastSystemError() for more info.
};

/** TODO */
enum class RequestStatus
{
	SUCCESS,                 ///< The request was succesful.
	NOT_CONNECTED,           ///< Request failed because the client is not connected. Call connect(...) first.
	SEND_REQUEST_FAILED,     ///< Failed to send the request message.
	CONNECTION_CLOSED,       ///< Server has closed the connection.
	NO_REPLY,                ///< No reply has arrived from the server in given timeout. In case this happens too often, you may try to increase the timeout.
	RECEIVE_ERROR,           ///< There has been some other error while trying to receive a reply. Call getLastSystemError() for more info.
	INVALID_REPLY            ///< The reply from the server is invalid.
};

enum class UpdateStatus
{
	UP_TO_DATE,              ///< The current device list seems up to date.
	OUT_OF_DATE,             ///< Server has sent a notification message indicating that the device list has changed. Call requestDeviceList() again.
	CONNECTION_CLOSED,       ///< Server has closed the connection.
	UNEXPECTED_MESSAGE,      ///< Server has sent some other kind of message that we didn't expect.
	CANT_RESTORE_SOCKET,     ///< Error has occured while trying to restore socket to its original state and the socket has been closed. Call getLastSystemError() for more info. This should never happen, but one never knows.
	OTHER_ERROR              ///< Other system error. Call getLastSystemError() for more info.
};

/** TODO */
struct DeviceListResult
{
	RequestStatus status;
	DeviceList devices;
};


//======================================================================================================================
/** TODO */

class Client
{

 public:

	/** Creates a client of specified or default name. Does not connect anywhere yet. */
	Client( const std::string & clientName = "orgb::Client" );

	~Client();

	/** Connects to the OpenRGB server and announces our client name. */
	ConnectStatus connect( const std::string & host = "127.0.0.1", uint16_t port = 6742 );

	/** Closes connection to the server. */
	void disconnect();

	bool isConnected() const;

	bool setTimeout( std::chrono::milliseconds timeout );

	/** Queries the server for information about all its RGB devices. */
	DeviceListResult requestDeviceList();

	UpdateStatus checkForDeviceUpdates();

	RequestStatus setDeviceColor( const Device & device, Color color );

	RequestStatus setZoneColor( const Zone & zone, Color color );

	RequestStatus setZoneSize( const Zone & zone, uint32_t newSize );

	RequestStatus setColorOfSingleLED( const LED & led, Color color );

	//RequestStatus modifyMode( const Mode & mode );

	//RequestStatus switchToCustomMode( const Device & device );

	system_error_t getLastSystemError() const;
	std::string getLastSystemErrorStr( system_error_t errorCode ) const     { return getErrorString( errorCode ); }
	std::string getLastSystemErrorStr() const             { return getLastSystemErrorStr( getLastSystemError() ); }

 private: // helpers

	template< typename Message, typename ... ConstructorArgs >
	bool sendMessage( ConstructorArgs ... args );

	template< typename Message >
	struct RecvResult
	{
		RequestStatus status;
		Message message;
	};
	template< typename Message >
	RecvResult< Message > awaitMessage();

	UpdateStatus hasUpdateMessageArrived();

 private:

	std::string _clientName;

	// pointer so that we don't have to include the TcpSocket and all its OS dependancies here
	std::unique_ptr< TcpClientSocket > _socket;

	bool _isDeviceListOutOfDate;

};


//======================================================================================================================


} // namespace orgb


#endif // OPENRGB_CLIENT_INCLUDED
