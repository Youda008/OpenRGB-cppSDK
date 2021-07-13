//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: serialization and deserialization of common parts of the protocol
//======================================================================================================================

#ifndef OPENRGB_PROTOCOL_COMMON_INCLUDED
#define OPENRGB_PROTOCOL_COMMON_INCLUDED


#include "ContainerUtils.hpp"
#include "BinaryStream.hpp"
MAKE_LITTLE_ENDIAN_DEFAULT

#include <string>
#include <vector>


namespace orgb {


//======================================================================================================================
// Putting these template functions into a struct allows us to collectively mark them as friend and allow them to access
// methods of Mode, Zone, LED that should be private to the user of the library, but accessible to the library itself.
// The struct is basically used as a "friendable" namespace here.

struct protocol
{

	//-- OpenRGB strings -----------------------------------------------------------------------------------------------

	static size_t sizeofString( const std::string & str ) noexcept
	{
		return 2 + str.size() + 1;
	}

	static void writeString( own::BinaryOutputStream & stream, const std::string & str )
	{
		stream << uint16_t( str.size() + 1 );
		stream.writeString0( str );
	}

	static bool readString( own::BinaryInputStream & stream, std::string & str ) noexcept
	{
		uint16_t size = 0;
		stream >> size;
		stream.readString0( str );
		return !stream.hasFailed() && str.size() + 1 == size;
	}


	//-- OpenRGB arrays ------------------------------------------------------------------------------------------------

	template< typename Type, typename std::enable_if< !std::is_trivial<Type>::value, int >::type = 0 >
	static size_t sizeofVectorWithDynamicObjects( const std::vector< Type > & vec, uint32_t protocolVersion ) noexcept
	{
		size_t size = 0;
		for (const Type & elem : vec)
		{
			size += elem.calcSize( protocolVersion );
		}
		return size;
	}

	template< typename Type, typename std::enable_if< std::is_trivial<Type>::value, int >::type = 0 >
	static size_t sizeofArray( const std::vector< Type > & vec ) noexcept
	{
		return 2 + own::sizeofVector( vec );
	}

	template< typename Type, typename std::enable_if< !std::is_trivial<Type>::value, int >::type = 0 >
	static size_t sizeofArray( const std::vector< Type > & vec, uint32_t protocolVersion ) noexcept
	{
		return 2 + sizeofVectorWithDynamicObjects( vec, protocolVersion );
	}

	template< typename Type, typename std::enable_if< std::is_trivial<Type>::value, int >::type = 0 >
	static void writeArray( own::BinaryOutputStream & stream, const std::vector< Type > vec )
	{
		stream << uint16_t(vec.size());
		for (const Type & elem : vec)
		{
			stream << elem;
		}
	}

	template< typename Type, typename std::enable_if< !std::is_trivial<Type>::value, int >::type = 0 >
	static void writeArray( own::BinaryOutputStream & stream, const std::vector< Type > vec, uint32_t protocolVersion )
	{
		stream << uint16_t(vec.size());
		for (const Type & elem : vec)
		{
			elem.serialize( stream, protocolVersion );
		}
	}

	template< typename Type, typename std::enable_if< std::is_trivial<Type>::value, int >::type = 0 >
	static bool readArray( own::BinaryInputStream & stream, std::vector< Type > & vec ) noexcept
	{
		uint16_t size = 0;
		stream >> size;
		vec.resize( size );
		for (uint16_t i = 0; i < size; ++i)
		{
			stream >> vec[i];
			if (stream.hasFailed())
				return false;
		}
		return !stream.hasFailed();
	}

	template< typename Type, typename std::enable_if< !std::is_trivial<Type>::value, int >::type = 0 >
	static bool readArray( own::BinaryInputStream & stream, std::vector< Type > & vec, uint32_t protocolVersion, uint32_t parentIdx ) noexcept
	{
		uint16_t size = 0;
		stream >> size;
		vec.reserve( size );
		for (uint16_t i = 0; i < size; ++i)
		{
			Type obj;
			if (!obj.deserialize( stream, protocolVersion, i, parentIdx ))
				return false;
			vec.emplace_back( move(obj) );
		}
		return !stream.hasFailed();
	}

};


//======================================================================================================================


} // namespace orgb


#endif // OPENRGB_PROTOCOL_COMMON_INCLUDED
