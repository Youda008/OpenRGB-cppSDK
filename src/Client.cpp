//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Created on:  1.11.2020
// Description: OpenRGB network client
//======================================================================================================================

#include "OpenRGB/Client.hpp"

#include "Common.hpp"

#include "OpenRGB/private/Protocol.hpp"
#include "OpenRGB/DeviceInfo.hpp"
#include "BufferStream.hpp"
#include "Socket.hpp"

#include <string>
using std::string;
#include <vector>
using std::vector;
#include <array>
using std::array;
#include <chrono>
using std::chrono::milliseconds;


namespace orgb {


//======================================================================================================================
//  Client: high-level API

Client::Client( const string & clientName )
:
	_clientName( clientName ),
	_socket( new TcpClientSocket ),
	_isDeviceListOutOfDate( true )
{}

Client::~Client() {}

ConnectStatus Client::connect( const string & host, uint16_t port )
{
	SocketError result1 = _socket->connect( host, port );
	if (result1 != SocketError::SUCCESS)
	{
		switch (result1)
		{
			case SocketError::ALREADY_CONNECTED:        return ConnectStatus::ALREADY_CONNECTED;
			case SocketError::NETWORKING_INIT_FAILED:   return ConnectStatus::NETWORKING_INIT_FAILED;
			case SocketError::HOST_NOT_RESOLVED:        return ConnectStatus::HOST_NOT_RESOLVED;
			case SocketError::CONNECT_FAILED:           return ConnectStatus::CONNECT_FAILED;
			default:                                    return ConnectStatus::OTHER_ERROR;
		}
	}

	// rather set some default timeout for recv operations, user can always override this
	_socket->setTimeout( milliseconds( 500 ) );

	bool result2 = sendMessage< SetClientName >( _clientName );
	if (!result2)
	{
		_socket->disconnect();  // let's be consistent, all other errors mean the connection was not opened
		return ConnectStatus::SEND_NAME_FAILED;
	}

	// The list isn't trully out of date, because there isn't any list yet. But let's say it is, because
	// it simplifies writing an application loop. This way user can just write
	//
	// while (true)
	// {
	//     if (!client.isConnected())
	//         client.connect();
	//     if (client.isDeviceListOutOfDate())
	//         deviceList = client.requestDeviceList();
	//     ...
	//     change colors
	//     ...
	// }
	_isDeviceListOutOfDate = true;

	return ConnectStatus::SUCCESS;
}

void Client::disconnect()
{
	_socket->disconnect();
}

bool Client::isConnected() const
{
	return _socket->isConnected();
}

bool Client::setTimeout( std::chrono::milliseconds timeout )
{
	// Currently we cannot set timeout on a socket that is not connected, because the actual system socket is created
	// during connect operation, so the preceeding setTimeout calls would go to nowhere.
	if (!_socket->isConnected())
	{
		return false;
	}

	return _socket->setTimeout( timeout );
}

DeviceListResult Client::requestDeviceList()
{
	if (!_socket->isConnected())
	{
		return { RequestStatus::NOT_CONNECTED, {} };
	}

	DeviceListResult result;

	do
	{
		result.devices.clear();
		_isDeviceListOutOfDate = false;

		bool sent = sendMessage< RequestControllerCount >();
		if (!sent)
		{
			result.status = RequestStatus::SEND_REQUEST_FAILED;
			return result;
		}

		auto deviceCountResult = awaitMessage< ReplyControllerCount >();
		if (deviceCountResult.status != RequestStatus::SUCCESS)
		{
			result.status = deviceCountResult.status;
			return result;
		}

		for (uint32_t deviceIdx = 0; deviceIdx < deviceCountResult.message.count; ++deviceIdx)
		{
			sent = sendMessage< RequestControllerData >( deviceIdx );
			if (!sent)
			{
				result.status = RequestStatus::SEND_REQUEST_FAILED;
				return result;
			}

			auto deviceDataResult = awaitMessage< ReplyControllerData >();
			if (deviceDataResult.status != RequestStatus::SUCCESS)
			{
				result.status = deviceDataResult.status;
				return result;
			}

			result.devices.append( deviceIdx, move( deviceDataResult.message.device_desc ) );
		}
	}
	// In the middle of the update we might receive DeviceListUpdated message. In that case we need to start again.
	while (_isDeviceListOutOfDate);

	result.status = RequestStatus::SUCCESS;
	return result;
}

/*RequestStatus Client::modifyMode( const Mode & mode )
{
	if (!_socket->isConnected())
	{
		return RequestStatus::NOT_CONNECTED;
	}

	if (!sendMessage< UpdateMode >( mode.parent.id, mode.id, mode.desc ))
	{
		return RequestStatus::SEND_REQUEST_FAILED;
	}

	return RequestStatus::SUCCESS;
}*/

RequestStatus Client::switchToDirectMode( const Device & device )
{
	if (!_socket->isConnected())
	{
		return RequestStatus::NOT_CONNECTED;
	}

	if (!sendMessage< SetCustomMode >( device.id ))
	{
		return RequestStatus::SEND_REQUEST_FAILED;
	}

	return RequestStatus::SUCCESS;
}

RequestStatus Client::setDeviceColor( const Device & device, Color color )
{
	if (!_socket->isConnected())
	{
		return RequestStatus::NOT_CONNECTED;
	}

	std::vector< Color > allColorsInDevice( device.leds.size(), color );
	if (!sendMessage< UpdateLEDs >( device.id, allColorsInDevice ))
	{
		return RequestStatus::SEND_REQUEST_FAILED;
	}

	return RequestStatus::SUCCESS;
}

RequestStatus Client::setZoneColor( const Zone & zone, Color color )
{
	if (!_socket->isConnected())
	{
		return RequestStatus::NOT_CONNECTED;
	}

	std::vector< Color > allColorsInZone( zone.numLeds, color );
	if (!sendMessage< UpdateZoneLEDs >( zone.parent.id, zone.id, allColorsInZone ))
	{
		return RequestStatus::SEND_REQUEST_FAILED;
	}

	return RequestStatus::SUCCESS;
}

RequestStatus Client::setZoneSize( const Zone & zone, uint32_t newSize )
{
	if (!_socket->isConnected())
	{
		return RequestStatus::NOT_CONNECTED;
	}

	if (!sendMessage< ResizeZone >( zone.parent.id, zone.id, newSize ))
	{
		return RequestStatus::SEND_REQUEST_FAILED;
	}

	return RequestStatus::SUCCESS;
}

RequestStatus Client::setColorOfSingleLED( const LED & led, Color color )
{
	if (!_socket->isConnected())
	{
		return RequestStatus::NOT_CONNECTED;
	}

	if (!sendMessage< UpdateSingleLED >( led.parent.id, led.id, color ))
	{
		return RequestStatus::SEND_REQUEST_FAILED;
	}

	return RequestStatus::SUCCESS;
}

UpdateStatus Client::checkForDeviceUpdates()
{
	if (_isDeviceListOutOfDate)
	{
		// Last time we found DeviceListUpdated message in the socket, and user haven't requested the new list yet,
		// no need to look again, keep reporting "out of date" until he calls requestDeviceList().
		return UpdateStatus::OUT_OF_DATE;
	}

	// Last time we checked there wasn't any DeviceListUpdated message, but it already might be now, so let's check.
	UpdateStatus status = hasUpdateMessageArrived();
	if (status == UpdateStatus::OUT_OF_DATE)
	{
		// DeviceListUpdated message found, cache this discovery until user calls requestDeviceList().
		_isDeviceListOutOfDate = true;
	}

	return status;
}

UpdateStatus Client::hasUpdateMessageArrived()
{
	// We only need to check if there is any TCP message in the system input buffer, but don't wait for it.
	// So we switch the socket to non-blocking mode and try to receive.

	if (!_socket->setBlockingMode( false ))
	{
		return UpdateStatus::OTHER_ERROR;
	}

	auto enableBlockingAndReturn = [ this ]( UpdateStatus returnStatus )
	{
		if (!_socket->setBlockingMode( true ))
		{
			// This is bad, we changed the state of the socket and now we're unable to return it back.
			// So rather burn everything to the ground and start from the beginning, than let things be in undefined state.
			disconnect();
			return UpdateStatus::CANT_RESTORE_SOCKET;
		}
		else
		{
			return returnStatus;
		}
	};

	vector< uint8_t > buffer;
	SocketError status = _socket->receive( buffer, Header::size() );
	if (status == SocketError::WOULD_BLOCK)
	{
		// No message is currently in the socket, no indication that the device list is out of date.
		return enableBlockingAndReturn( UpdateStatus::UP_TO_DATE );
	}
	else if (status == SocketError::CONNECTION_CLOSED)
	{
		return enableBlockingAndReturn( UpdateStatus::CONNECTION_CLOSED );
	}
	else if (status != SocketError::SUCCESS)
	{
		return enableBlockingAndReturn( UpdateStatus::OTHER_ERROR );
	}

	// We have some message, so let's check what it is.

	Header header;
	BufferInputStream stream( move( buffer ) );
	if (!header.deserialize( stream ) || header.message_type != MessageType::DEVICE_LIST_UPDATED)
	{
		// We received something, but something totally different than what we expected.
		return enableBlockingAndReturn( UpdateStatus::UNEXPECTED_MESSAGE );
	}
	else
	{
		// We have received a DeviceListUpdated message from the server,
		// signal to the user that he needs to request the list again.
		return enableBlockingAndReturn( UpdateStatus::OUT_OF_DATE );
	}
}


//======================================================================================================================
//  helpers

template< typename Message, typename ... ConstructorArgs >
bool Client::sendMessage( ConstructorArgs ... args )
{
	Message message( args ... );

	BufferOutputStream stream;
	stream.reserveAdditional( message.header.size() + message.header.message_size );
	message.serialize( stream );

	return _socket->send( stream.buffer() ) == SocketError::SUCCESS;
}

template< typename Message >
Client::RecvResult< Message > Client::awaitMessage()
{
	RecvResult< Message > result;
	vector< uint8_t > buffer;

	do
	{
		// receive header into buffer
		SocketError headerStatus = _socket->receive( buffer, Header::size() );
		if (headerStatus != SocketError::SUCCESS)
		{
			switch (headerStatus)
			{
				case SocketError::CONNECTION_CLOSED:  result.status = RequestStatus::CONNECTION_CLOSED;  break;
				case SocketError::TIMEOUT:            result.status = RequestStatus::NO_REPLY;           break;
				default:                              result.status = RequestStatus::RECEIVE_ERROR;      break;
			}
			return result;
		}

		// parse and validate the header
		BufferInputStream stream( move( buffer ) );
		if (!result.message.header.deserialize( stream ))
		{
			result.status = RequestStatus::INVALID_REPLY;
			return result;
		}

		// the server may have sent DeviceListUpdated messsage before it received our request
		if (result.message.header.message_type == MessageType::DEVICE_LIST_UPDATED)
		{
			// in that case just set our "out of date" flag and skip it for now
			_isDeviceListOutOfDate = true;
		}
	}
	while (result.message.header.message_type == MessageType::DEVICE_LIST_UPDATED);

	if (result.message.header.message_type != Message::thisType)
	{
		// the message is neither DeviceListUpdated, nor the type we expected
		result.status = RequestStatus::INVALID_REPLY;
		return result;
	}

	// receive the message body
	SocketError bodyStatus = _socket->receive( buffer, result.message.header.message_size );
	if (bodyStatus != SocketError::SUCCESS)
	{
		switch (bodyStatus)
		{
			case SocketError::CONNECTION_CLOSED:  result.status = RequestStatus::CONNECTION_CLOSED;  break;
			case SocketError::TIMEOUT:            result.status = RequestStatus::NO_REPLY;           break;
			default:                              result.status = RequestStatus::RECEIVE_ERROR;      break;
		}
		return result;
	}

	// parse and validate the body
	BufferInputStream stream( move( buffer ) );
	if (!result.message.deserializeBody( stream ))
	{
		result.status = RequestStatus::INVALID_REPLY;
	}
	else
	{
		result.status = RequestStatus::SUCCESS;
	}

	return result;
}


//======================================================================================================================


} // namespace orgb
