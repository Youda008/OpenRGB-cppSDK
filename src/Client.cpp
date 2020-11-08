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
#include "OpenRGB/private/DeviceInfo.hpp"
#include "BufferStream.hpp"
#include "Socket.hpp"

#include <string>
using std::string;
#include <vector>
using std::vector;
#include <array>
using std::array;


namespace orgb {


//======================================================================================================================
//  Client: high-level API

Client::Client( const string & clientName )
	: _clientName( clientName ), _socket( new TcpClientSocket ) {}

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
			default:                                    return ConnectStatus::OTHER;
		}
	}

	bool result2 = sendMessage< SetClientName >( _clientName );
	if (!result2)
	{
		_socket->disconnect();  // let's be consistent, all other errors mean the connection was not opened
		return ConnectStatus::SEND_NAME_FAILED;
	}

	return ConnectStatus::SUCCESS;
}

void Client::disconnect()
{
	_socket->disconnect();
}

DeviceListResult Client::requestDeviceList()
{
	if (!_socket->isConnected())
	{
		return { RequestStatus::NOT_CONNECTED, {} };
	}

	bool sent = sendMessage< RequestControllerCount >();
	if (!sent)
	{
		return { RequestStatus::SEND_REQUEST_FAILED, {} };
	}

	auto deviceCountResult = awaitMessage< ReplyControllerCount >();
	if (deviceCountResult.status != RequestStatus::SUCCESS)
	{
		return { deviceCountResult.status, {} };
	}

	DeviceListResult result;

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

	result.status = RequestStatus::SUCCESS;
	return result;
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
}

RequestStatus Client::switchToCustomMode( const Device & device )
{
	if (!_socket->isConnected())
	{
		return RequestStatus::NOT_CONNECTED;
	}

	if (!sendMessage< RequestControllerCount >())
	{
		return RequestStatus::SEND_REQUEST_FAILED;
	}

	return RequestStatus::SUCCESS;
}*/

bool Client::isDeviceListOutdated()
{
	// TODO: peak message from socket and check if it isn't DeviceListUpdated
	return false;
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
	if (!result.message.header.deserialize( stream ) || result.message.header.message_type != Message::thisType)
	{
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
	stream.reset( move( buffer ) );
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
