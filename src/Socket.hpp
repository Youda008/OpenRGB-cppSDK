//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Created on:  1.11.2020
// Description: abstraction over low-level system socket calls
//======================================================================================================================

#ifndef OPENRGB_SOCKET_INCLUDED
#define OPENRGB_SOCKET_INCLUDED


#include "Common.hpp"

#include "OpenRGB/private/SystemErrorInfo.hpp"

#ifdef _WIN32
	#define WIN32_LEAN_AND_MEAN
	#include <winsock2.h>
#else
	#include <sys/socket.h>
#endif // _WIN32

#include <chrono>  // timeout
#include <vector>  // send, recv
#include <array>


namespace orgb {


//======================================================================================================================
//  types shared between multiple socket classes

/** our own unified error codes, because error codes from system calls vary from OS to OS */
enum class SocketError
{
	SUCCESS = 0,                   ///< The operation was successful.
	// our own error states that don't have anything to do with the system sockets
	ALREADY_CONNECTED = 1,         ///< Connect operation failed because the socket is already connected. Call disconnect() first.
	NOT_CONNECTED = 2,             ///< Operation failed because the socket is not connected. Call connect(...) first.
	// errors related to connect attempt
	NETWORKING_INIT_FAILED = 10,   ///< Operation failed because underlying networking system could not be initialized. Call getLastSystemError() for more info.
	HOST_NOT_RESOLVED = 11,        ///< The hostname you entered could not be resolved to IP address. Call getLastSystemError() for more info.
	CONNECT_FAILED = 12,           ///< Could not connect to the target server, either it's down or the port is closed. Call getLastSystemError() for more info.
	// errors related to send operation
	SEND_FAILED = 20,              ///< Send operation failed. Call getLastSystemError() for more info.
	// errors related to receive operation
	CONNECTION_CLOSED = 30,        ///< Server has closed the connection.
	TIMEOUT = 31,                  ///< Operation timed-out.
	WOULD_BLOCK = 32,              ///< Socket is set to non-blocking mode and there is no data in the system input buffer.

	OTHER = 255                    ///< Other system error. Call getLastSystemError() for more info.
};

#ifdef _WIN32
	using socket_t = SOCKET;
#else
	using socket_t = int;
#endif


//======================================================================================================================
/** common for all socket classes */

class _SocketCommon
{

 public:

	system_error_t getLastSystemError() const   { return _lastSystemError; }
	bool setBlockingMode( bool enable )         { _isBlocking = enable; return _setBlockingMode( _socket, enable ); }
	bool isBlocking() const                     { return _isBlocking; }

 protected:

	_SocketCommon();

	static bool _shutdownSocket( socket_t sock );
	static bool _closeSocket( socket_t sock );
	static bool _setTimeout( socket_t sock, std::chrono::milliseconds timeout );
	static bool _isTimeout( system_error_t errorCode );
	static bool _isWouldBlock( system_error_t errorCode );
	static bool _setBlockingMode( socket_t sock, bool enable );

 protected:

	socket_t _socket;
	system_error_t _lastSystemError;
	bool _isBlocking;

};


//======================================================================================================================
/** abstraction over low-level TCP socket system calls */

class TcpClientSocket : public _SocketCommon
{

 public:

	TcpClientSocket();
	~TcpClientSocket();

	SocketError connect( const std::string & host, uint16_t port );
	SocketError disconnect();
	bool isConnected() const;

	bool setTimeout( std::chrono::milliseconds timeout );

	/** Sends given number of bytes to the socket. If the system does not accept that amount of data all at once,
	  * it repeats the system calls until all requested data are sent. */
	SocketError send( const uint8_t * buffer, size_t size );
	SocketError send( const std::vector< uint8_t > & buffer )       { return send( buffer.data(), buffer.size() ); }
	template< size_t size >
	SocketError send( const std::array< uint8_t, size > & buffer )  { return send( buffer.data(), buffer.size() ); }

	/** Receive the given number of bytes from the socket. If the requested amount of data don't arrive all at once,
	  * it repeats the system calls until all requested data are received.
	  * @param[in, out] size - it needs to be set to desired size before call,
	  *                        and will contain actual received size after the call */
	SocketError receive( uint8_t * buffer, size_t & size );
	SocketError receive( std::vector< uint8_t > & buffer, size_t size )
	{
		buffer.resize( size );  // allocate the needed storage
		SocketError result = receive( buffer.data(), size );
		buffer.resize( size );  // let's return the user a vector only as big as how much we actually received
		return result;
	}
	template< size_t size >
	SocketError receive( std::array< uint8_t, size > & buffer, size_t & received )
	{
		received = buffer.size();
		return receive( buffer.data(), received );
	}

};


//======================================================================================================================
/** TODO */

class UdpSocket : public _SocketCommon
{

 public:

	UdpSocket();
	~UdpSocket();

	// TODO

};


//======================================================================================================================


} // namespace orgb


#endif // OPENRGB_SOCKET_INCLUDED
