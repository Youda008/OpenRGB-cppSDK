//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: serialization and deserialization of common parts of the protocol
//======================================================================================================================

#ifndef OPENRGB_PROTOCOL_COMMON_INCLUDED
#define OPENRGB_PROTOCOL_COMMON_INCLUDED


#include "BinaryStream.hpp"
MAKE_LITTLE_ENDIAN_DEFAULT

#include <string>
#include <vector>


namespace orgb {


//======================================================================================================================
//  OpenRGB strings

inline size_t sizeofORGBString( const std::string & str )
{
	return 2 + str.size() + 1;
}

inline void writeORGBString( own::BinaryOutputStream & stream, const std::string & str )
{
	stream << uint16_t(str.size());
	stream.writeString0( str );
}

inline bool readORGBString( own::BinaryInputStream & stream, std::string & str )
{
	uint16_t size = 0;
	stream >> size;
	stream.readString0( str );
	return !stream.hasFailed() && str.size() == size;
}


//======================================================================================================================
//  OpenRGB arrays

template< typename Type, typename std::enable_if< std::is_fundamental<Type>::value, int >::type = 0 >
size_t sizeofVector( const std::vector< Type > & vec )
{
	return vec.size() * sizeof( Type );
}

template< typename Type, typename std::enable_if< !std::is_fundamental<Type>::value, int >::type = 0 >
size_t sizeofVector( const std::vector< Type > & vec )
{
	size_t size = 0;
	for (const Type & elem : vec)
	{
		size += elem.calcSize();
	}
	return size;
}

template< typename Type >
size_t sizeofORGBArray( const std::vector< Type > & vec )
{
	return 2 + sizeofVector( vec );
}

template< typename Type, typename std::enable_if< std::is_integral<Type>::value, int >::type = 0 >
void writeORGBArray( own::BinaryOutputStream & stream, const std::vector< Type > vec )
{
	stream << uint16_t(vec.size());
	for (const Type & elem : vec)
	{
		stream << elem;
	}
}

template< typename Type, typename std::enable_if< !std::is_integral<Type>::value, int >::type = 0 >
void writeORGBArray( own::BinaryOutputStream & stream, const std::vector< Type > vec )
{
	stream << uint16_t(vec.size());
	for (const Type & elem : vec)
	{
		elem.serialize( stream );
	}
}

template< typename Type, typename std::enable_if< std::is_integral<Type>::value, int >::type = 0 >
bool readORGBArray( own::BinaryInputStream & stream, std::vector< Type > & vec )
{
	uint16_t size = 0;
	stream >> size;
	vec.resize( size );
	for (uint16_t i = 0; i < size; ++i)
	{
		stream >> vec[i];
	}
	return !stream.hasFailed();
}

template< typename Type, typename std::enable_if< !std::is_integral<Type>::value, int >::type = 0 >
bool readORGBArray( own::BinaryInputStream & stream, std::vector< Type > & vec )
{
	uint16_t size = 0;
	stream >> size;
	vec.resize( size );
	for (uint16_t i = 0; i < size; ++i)
	{
		vec[i].deserialize( stream );
	}
	return !stream.hasFailed();
}


//======================================================================================================================


} // namespace orgb


#endif // OPENRGB_PROTOCOL_COMMON_INCLUDED
