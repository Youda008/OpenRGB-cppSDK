//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: declaration of the protocol messages and types
//======================================================================================================================

#include "ProtocolMessages.hpp"

#include "ProtocolCommon.hpp"
#include "BinaryStream.hpp"
using own::BinaryOutputStream;
using own::BinaryInputStream;

#include <cstring>  // strncmp
#include <string>
using std::string;
#include <vector>
using std::vector;


namespace orgb {


//======================================================================================================================
//  message header

const char * enumString( MessageType type ) noexcept
{
	// the values of message types are wildly different so we can't use an array
	switch (type)
	{
		case MessageType::REQUEST_CONTROLLER_COUNT:       return "REQUEST_CONTROLLER_COUNT";
		case MessageType::REQUEST_CONTROLLER_DATA:        return "REQUEST_CONTROLLER_DATA";
		case MessageType::REQUEST_PROTOCOL_VERSION:       return "REQUEST_PROTOCOL_VERSION";
		case MessageType::SET_CLIENT_NAME:                return "SET_CLIENT_NAME";
		case MessageType::DEVICE_LIST_UPDATED:            return "DEVICE_LIST_UPDATED";
		case MessageType::RGBCONTROLLER_RESIZEZONE:       return "RGBCONTROLLER_RESIZEZONE";
		case MessageType::RGBCONTROLLER_UPDATELEDS:       return "RGBCONTROLLER_UPDATELEDS";
		case MessageType::RGBCONTROLLER_UPDATEZONELEDS:   return "RGBCONTROLLER_UPDATEZONELEDS";
		case MessageType::RGBCONTROLLER_UPDATESINGLELED:  return "RGBCONTROLLER_UPDATESINGLELED";
		case MessageType::RGBCONTROLLER_SETCUSTOMMODE:    return "RGBCONTROLLER_SETCUSTOMMODE";
		case MessageType::RGBCONTROLLER_UPDATEMODE:       return "RGBCONTROLLER_UPDATEMODE";
		default:                                          return "<invalid>";
	}
}

static bool isValidMessageType( MessageType type )
{
	switch (type)
	{
		case MessageType::REQUEST_CONTROLLER_COUNT:      return true;
		case MessageType::REQUEST_CONTROLLER_DATA:       return true;
		case MessageType::REQUEST_PROTOCOL_VERSION:      return true;
		case MessageType::SET_CLIENT_NAME:               return true;
		case MessageType::DEVICE_LIST_UPDATED:           return true;
		case MessageType::RGBCONTROLLER_RESIZEZONE:      return true;
		case MessageType::RGBCONTROLLER_UPDATELEDS:      return true;
		case MessageType::RGBCONTROLLER_UPDATEZONELEDS:  return true;
		case MessageType::RGBCONTROLLER_UPDATESINGLELED: return true;
		case MessageType::RGBCONTROLLER_SETCUSTOMMODE:   return true;
		case MessageType::RGBCONTROLLER_UPDATEMODE:      return true;
		default:                                         return false;
	}
}

void Header::serialize( BinaryOutputStream & stream ) const
{
	stream << magic[0] << magic[1] << magic[2] << magic[3];
	stream << device_idx;
	stream << message_type;
	stream << message_size;
}

bool Header::deserialize( BinaryInputStream & stream ) noexcept
{
	stream >> magic[0] >> magic[1] >> magic[2] >> magic[3];
	stream >> device_idx;
	stream >> message_type;
	stream >> message_size;

	if (strncmp( magic, "ORGB", sizeof(magic) ) != 0)
		stream.setFailed();
	if (!isValidMessageType( message_type ))
		stream.setFailed();

	return !stream.hasFailed();
}


//======================================================================================================================
//  main protocol messages

//----------------------------------------------------------------------------------------------------------------------

void ReplyControllerCount::serialize( BinaryOutputStream & stream ) const
{
	header.serialize( stream );

	stream << count;
}

bool ReplyControllerCount::deserializeBody( BinaryInputStream & stream ) noexcept
{
	stream >> count;

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

void RequestControllerData::serialize( own::BinaryOutputStream & stream ) const
{
	header.serialize( stream );

	stream << protocolVersion;
}

bool RequestControllerData::deserializeBody( own::BinaryInputStream & stream ) noexcept
{
	stream >> protocolVersion;

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t ReplyControllerData::calcDataSize() const noexcept
{
	size_t size = 0;

	size += sizeof( data_size );
	size += device_desc.calcSize();

	return uint32_t( size );
}

void ReplyControllerData::serialize( BinaryOutputStream & stream ) const
{
	header.serialize( stream );

	stream << data_size;
	device_desc.serialize( stream );
}

bool ReplyControllerData::deserializeBody( BinaryInputStream & stream ) noexcept
{
	stream >> data_size;
	device_desc.deserialize( stream, header.device_idx );

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

void RequestProtocolVersion::serialize( BinaryOutputStream & stream ) const
{
	header.serialize( stream );

	stream << clientVersion;
}

bool RequestProtocolVersion::deserializeBody( BinaryInputStream & stream ) noexcept
{
	stream >> clientVersion;

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

void ReplyProtocolVersion::serialize( BinaryOutputStream & stream ) const
{
	header.serialize( stream );

	stream << serverVersion;
}

bool ReplyProtocolVersion::deserializeBody( BinaryInputStream & stream ) noexcept
{
	stream >> serverVersion;

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t SetClientName::calcDataSize() const noexcept
{
	size_t size = 0;

	size += name.size() + 1;

	return uint32_t( size );
}

void SetClientName::serialize( BinaryOutputStream & stream ) const
{
	header.serialize( stream );

	stream.writeString0( name );
}

bool SetClientName::deserializeBody( BinaryInputStream & stream ) noexcept
{
	stream.readString0( name );

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

void ResizeZone::serialize( BinaryOutputStream & stream ) const
{
	header.serialize( stream );

	stream << zone_idx;
	stream << new_size;
}

bool ResizeZone::deserializeBody( BinaryInputStream & stream ) noexcept
{
	stream >> zone_idx;
	stream >> new_size;

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t UpdateLEDs::calcDataSize() const noexcept
{
	size_t size = 0;

	size += sizeof( data_size );
	size += protocol::sizeofArray( colors );

	return uint32_t( size );
}

void UpdateLEDs::serialize( BinaryOutputStream & stream ) const
{
	header.serialize( stream );

	stream << data_size;
	protocol::writeArray( stream, colors );
}

bool UpdateLEDs::deserializeBody( BinaryInputStream & stream ) noexcept
{
	stream >> data_size;
	protocol::readArray( stream, colors );

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t UpdateZoneLEDs::calcDataSize() const noexcept
{
	size_t size = 0;

	size += sizeof( data_size );
	size += sizeof( zone_idx );
	size += protocol::sizeofArray( colors );

	return uint32_t( size );
}

void UpdateZoneLEDs::serialize( BinaryOutputStream & stream ) const
{
	header.serialize( stream );

	stream << data_size;
	stream << zone_idx;
	protocol::writeArray( stream, colors );
}

bool UpdateZoneLEDs::deserializeBody( BinaryInputStream & stream ) noexcept
{
	stream >> data_size;
	stream >> zone_idx;
	protocol::readArray( stream, colors );

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t UpdateSingleLED::calcDataSize() const noexcept
{
	size_t size = 0;

	size += sizeof( led_idx );
	size += color.calcSize();

	return uint32_t( size );
}

void UpdateSingleLED::serialize( BinaryOutputStream & stream ) const
{
	header.serialize( stream );

	stream << led_idx;
	stream << color;
}

bool UpdateSingleLED::deserializeBody( BinaryInputStream & stream ) noexcept
{
	stream >> led_idx;
	stream >> color;

	return !stream.hasFailed();
}

//----------------------------------------------------------------------------------------------------------------------

uint32_t UpdateMode::calcDataSize() const noexcept
{
	size_t size = 0;

	size += sizeof( data_size );
	size += sizeof( mode_idx );
	size += mode_desc.calcSize();

	return uint32_t( size );
}

void UpdateMode::serialize( BinaryOutputStream & stream ) const
{
	header.serialize( stream );

	stream << data_size;
	stream << mode_idx;
	mode_desc.serialize( stream );
}

bool UpdateMode::deserializeBody( BinaryInputStream & stream ) noexcept
{
	stream >> data_size;
	stream >> mode_idx;
	mode_desc.deserialize( stream, mode_idx, header.device_idx );

	return !stream.hasFailed();
}


//======================================================================================================================


} // namespace orgb
