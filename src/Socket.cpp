//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Created on:  1.11.2020
// Description: abstraction over low-level system socket calls
//======================================================================================================================

#include "Socket.hpp"

#include "Utils.hpp"

#ifdef _WIN32
	#include <winsock2.h>      // socket, closesocket
	#include <ws2tcpip.h>      // addrinfo

	using in_addr_t = unsigned long;  // windows does not use in_addr_t, it uses unsigned long instead
	using socklen_t = int;

	constexpr orgb::socket_t INVALID_SOCK = INVALID_SOCKET;
	constexpr orgb::system_error_t SUCCESS = ERROR_SUCCESS;
#else
	#include <unistd.h>        // open, close, read, write
	#include <sys/socket.h>    // socket
	#include <netdb.h>         // getaddrinfo, gethostbyname
	#include <netinet/in.h>    // sockaddr_in, in_addr, ntoh, hton
	#include <arpa/inet.h>     // inet_addr, inet_ntoa

	constexpr orgb::socket_t INVALID_SOCK = -1;
	constexpr orgb::SystemError SUCCESS = 0;
#endif // _WIN32

#include <mutex>


namespace orgb {


//======================================================================================================================
//  network subsystem initialization and automatic termination

/** private class for internal use */
class _NetworkingSubsystem
{

 public:

	_NetworkingSubsystem() : _initialized( false ) {}

	bool initializeIfNotAlready()
	{
		// this additional check is optimization for cases where it's already initialized, which is gonna be the majority
		if (_initialized)
			return true;

		// this method may get called from multiple threads, so we need to make sure they don't race
		std::unique_lock< std::mutex > lock( mtx );
		if (!_initialized)
		{
			_initialized = _initialize();
		}
		return _initialized;
	}

	~_NetworkingSubsystem()
	{
		if (_initialized)
		{
			_terminate();
		}
	}

 private:

	bool _initialize()
	{
 #ifdef _WIN32
		return WSAStartup( MAKEWORD(2, 2), &_wsaData ) == NO_ERROR;
 #else
		return true;
 #endif // _WIN32
	}

	void _terminate()
	{
 #ifdef _WIN32
		WSACleanup();
 #else
 #endif // _WIN32
	}

 private:

 #ifdef _WIN32
	WSADATA _wsaData;
 #else
 #endif // _WIN32

	std::mutex mtx;
	bool _initialized;

};

static _NetworkingSubsystem g_netSystem;  // this will get initialized on first use and terminated on process exit


//======================================================================================================================
//  SocketCommon

_SocketCommon::_SocketCommon()
:
	_socket( INVALID_SOCK ),
	_lastSystemError( SUCCESS ),
	_isBlocking( true )
{}

bool _SocketCommon::_shutdownSocket( socket_t sock )
{
 #ifdef _WIN32
	return shutdown( sock, SD_BOTH ) != 0;
 #else
	return shutdown( sock, SHUT_RDWR ) != 0;
 #endif // _WIN32
}

bool _SocketCommon::_closeSocket( socket_t sock )
{
 #ifdef _WIN32
	return closesocket( sock ) != 0;
 #else
	return close( sock ) != 0;
 #endif // _WIN32
}

bool _SocketCommon::_setTimeout( socket_t sock, std::chrono::milliseconds timeout_ms )
{
 #ifdef _WIN32
	DWORD timeout = (DWORD)timeout_ms.count();
 #else
	struct timeval timeout;
	timeout.tv_sec  = timeout_ms.count() / 1000;
	timeout.tv_usec = (timeout_ms.count() % 1000) * 1000;
 #endif // _WIN32

	return setsockopt( sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout) ) == 0;
}

bool _SocketCommon::_isTimeout( system_error_t errorCode )
{
 #ifdef _WIN32
	return errorCode == WSAETIMEDOUT;
 #else
	return errorCode == EAGAIN || errorCode == EWOULDBLOCK || errorCode == ETIMEDOUT;
 #endif // _WIN32
}

bool _SocketCommon::_isWouldBlock( system_error_t errorCode )
{
 #ifdef _WIN32
	return errorCode == WSAEWOULDBLOCK;
 #else
	return errorCode == EAGAIN || errorCode == EWOULDBLOCK;
 #endif // _WIN32
}

bool _SocketCommon::_setBlockingMode( socket_t sock, bool enable )
{
#ifdef _WIN32
	unsigned long mode = enable ? 0 : 1;
	return ioctlsocket( sock, (long)FIONBIO, &mode ) == 0;
#else
	int flags = fcntl( sock, F_GETFL, 0 );
	if (flags == -1)
		return false;

	if (enable)
		flags &= ~O_NONBLOCK;
	else
		flags |= O_NONBLOCK;

	return fcntl( sock, F_SETFL, flags ) == 0;
#endif
}


//======================================================================================================================
//  TcpSocket

TcpClientSocket::TcpClientSocket() : _SocketCommon() {}

TcpClientSocket::~TcpClientSocket()
{
	disconnect();
}

SocketError TcpClientSocket::connect( const std::string & host, uint16_t port )
{
	if (_socket != INVALID_SOCK)
	{
		return SocketError::ALREADY_CONNECTED;
	}

	bool initialized = g_netSystem.initializeIfNotAlready();
	if (!initialized)
	{
		_lastSystemError = getLastSystemError();
		return SocketError::NETWORKING_INIT_FAILED;
	}

	char portStr [8];
	snprintf( portStr, sizeof(portStr), "%hu", ushort(port) );

	struct addrinfo hint;
	memset( &hint, 0, sizeof(hint) );
	hint.ai_family = AF_UNSPEC;      // IPv4 or IPv6, it doesn't matter
	hint.ai_socktype = SOCK_STREAM;  // but only TCP!

	// find protocol family and address of the host
	struct addrinfo * ainfo;
	if (getaddrinfo( host.c_str(), portStr, &hint, &ainfo ) != SUCCESS)
	{
		_lastSystemError = getLastSystemError();
		return SocketError::HOST_NOT_RESOLVED;
	}
	auto ainfo_guard = at_scope_end_do( [ &ainfo ]() { freeaddrinfo( ainfo ); } );

	// create a corresponding socket
	_socket = socket( ainfo->ai_family, ainfo->ai_socktype, ainfo->ai_protocol );
	if (_socket == INVALID_SOCK)
	{
		_lastSystemError = getLastSystemError();
		return SocketError::OTHER;
	}

	if (::connect( _socket, ainfo->ai_addr, int( ainfo->ai_addrlen ) ) != SUCCESS)
	{
		_closeSocket( _socket );
		_socket = INVALID_SOCK;
		_lastSystemError = getLastSystemError();
		return SocketError::CONNECT_FAILED;
	}

	_lastSystemError = getLastSystemError();
	return SocketError::SUCCESS;
}

SocketError TcpClientSocket::disconnect()
{
	if (_socket == INVALID_SOCK)
	{
		return SocketError::NOT_CONNECTED;
	}

	if (!_shutdownSocket( _socket ))
	{
		_socket = INVALID_SOCK;
		return SocketError::OTHER;
	}

	if (!_closeSocket( _socket ))
	{
		_socket = INVALID_SOCK;
		return SocketError::OTHER;
	}

	_socket = INVALID_SOCK;
	return SocketError::SUCCESS;
}

bool TcpClientSocket::isConnected() const
{
	return _socket != INVALID_SOCK;
}

bool TcpClientSocket::setTimeout( std::chrono::milliseconds timeout )
{
	return _setTimeout( _socket, timeout );
}

SocketError TcpClientSocket::send( const uint8_t * buffer, size_t size )
{
	if (_socket == INVALID_SOCK)
	{
		return SocketError::NOT_CONNECTED;
	}

	const uint8_t * sendBegin = buffer;
	size_t sendSize = size;
	while (sendSize > 0)
	{
		int sent = ::send( _socket, (const char *)sendBegin, int( sendSize ), 0 );
		if (sent < 0)
		{
			_lastSystemError = getLastError();
			return SocketError::SEND_FAILED;
		}
		sendBegin += sent;
		sendSize -= size_t( sent );
	}
	return SocketError::SUCCESS;
}

SocketError TcpClientSocket::receive( uint8_t * buffer, size_t & size )
{
	if (_socket == INVALID_SOCK)
	{
		return SocketError::NOT_CONNECTED;
	}

	uint8_t * recvBegin = buffer;
	size_t recvSize = size;
	while (recvSize > 0)
	{
		int received = ::recv( _socket, (char *)recvBegin, int( recvSize ), 0 );
		if (received <= 0)
		{
			_lastSystemError = getLastError();
			size -= recvSize;  // this is how much we failed to receive

			if (received == 0)
			{
				_closeSocket( _socket );  // server closed, so let's close on our side too
				_socket = INVALID_SOCK;
				return SocketError::CONNECTION_CLOSED;
			}
			else if (!_isBlocking && _isWouldBlock( _lastSystemError ))
			{
				return SocketError::WOULD_BLOCK;
			}
			else if (_isTimeout( _lastSystemError ))
			{
				return SocketError::TIMEOUT;
			}
			else
			{
				return SocketError::OTHER;
			}
		}
		recvBegin += received;
		recvSize -= size_t( received );
	}
	return SocketError::SUCCESS;
}


//======================================================================================================================
//  UdpSocket

UdpSocket::UdpSocket() : _SocketCommon() {}


//======================================================================================================================


} // namespace orgb
