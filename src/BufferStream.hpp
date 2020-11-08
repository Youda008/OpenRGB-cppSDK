//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Created on:  2.11.2020
// Description: classes for binary serialization into binary buffers via operators << and >>
//======================================================================================================================

#ifndef OPENRGB_BUFFER_STREAM_INCLUDED
#define OPENRGB_BUFFER_STREAM_INCLUDED


#include "Common.hpp"

#include "Utils.hpp"

#include <vector>
#include <string>
#include <algorithm>  // copy


namespace orgb {

// TODO: allow deserialization from static array to prevent allocation in case of simple static messages

//======================================================================================================================
/** binary buffer input stream allowing serialization via operator << */

class BufferOutputStream
{

	using byteVecIter = std::vector<uint8_t>::iterator;

 public:

	BufferOutputStream() {}
	~BufferOutputStream() {}

	const std::vector< uint8_t > & buffer() const   { return _buffer; }

	void clear()                                    { _buffer.clear(); }

	void reserve( size_t size )                     { _buffer.reserve( size ); }
	void reserveAdditional( size_t additionalSize ) { _buffer.reserve( _buffer.size() + additionalSize ); }

	BufferOutputStream & operator<<( uint8_t b )
	{
		_buffer.push_back( b );
		return *this;
	}

	BufferOutputStream & operator<<( char c )
	{
		_buffer.push_back( uint8_t(c) );
		return *this;
	}

	/** Writes a fundamental type to the buffer, maintaining its host-native byte order (no endianity conversion for integers). */
	template< typename Type, typename std::enable_if< std::is_fundamental<Type>::value, int >::type = 0 >
	BufferOutputStream & operator<<( Type x )
	{
		_buffer.resize( _buffer.size() + sizeof(x) );
		std::copy( (uint8_t *)(&x), (uint8_t *)(&x + 1), _buffer.end() - sizeof(x) );
		return *this;
	}

	/** Writes a string and its null terminator to the buffer. */
	BufferOutputStream & operator<<( const std::string & str )
	{
		writeString0( str );
		return *this;
	}

	/** Converts an arbitrary integral number from native format to big endian and writes it into the buffer. */
	template< typename IntType, typename std::enable_if< std::is_integral<IntType>::value, int >::type = 0 >
	void writeIntBE( IntType native )
	{
		_buffer.resize( _buffer.size() + sizeof(IntType) );

		const byteVecIter bigEndianEnd = _buffer.end();
		const byteVecIter bigEndianBegin = bigEndianEnd - sizeof(IntType);

		byteVecIter pos = bigEndianEnd;
		while (pos > bigEndianBegin) { // can't use traditional for-loop approach, because decrementing past begin
			--pos;                     // is formally undefined and causes assertion in some standard lib implementations
			*pos = uint8_t(native & 0xFF);
			native = IntType(native >> 8);
		}
	}

	/** Converts an arbitrary integral number from native format to little endian and writes it into the buffer. */
	template< typename IntType, typename std::enable_if< std::is_integral<IntType>::value, int >::type = 0 >
	void writeIntLE( IntType native )
	{
		_buffer.resize( _buffer.size() + sizeof(IntType) );

		const byteVecIter littleEndianEnd = _buffer.end();
		const byteVecIter littleEndianBegin = littleEndianEnd - sizeof(IntType);

		byteVecIter pos = littleEndianBegin;
		while (pos < littleEndianEnd) {
			*pos = uint8_t(native & 0xFF);
			native = IntType(native >> 8);
			++pos;
		}
	}

	/** Converts an integer representation of an enum value from native format to big endian and writes it into the buffer. */
	template< typename EnumType, typename std::enable_if< std::is_enum<EnumType>::value, int >::type = 0 >
	void writeEnumBE( EnumType native )
	{
		writeIntBE( enumToInt( native ) );
	}

	/** Converts an integer representation of an enum value from native format to little endian and writes it into the buffer. */
	template< typename EnumType, typename std::enable_if< std::is_enum<EnumType>::value, int >::type = 0 >
	void writeEnumLE( EnumType native )
	{
		writeIntLE( enumToInt( native ) );
	}

	/** Writes a string WITHOUT its null terminator to the buffer. */
	void writeString( const std::string & str );

	/** Writes a string WITH its null terminator to the buffer. */
	void writeString0( const std::string & str );

	// TODO: write arbitrary number of characters or bytes

 private:

	std::vector< uint8_t > _buffer;

};


//======================================================================================================================
/** binary buffer input stream allowing deserialization via operator >> */

class BufferInputStream
{

	using constByteVecIter = std::vector<uint8_t>::const_iterator;

 public:

	BufferInputStream( const std::vector< uint8_t > & buffer ) : _buffer( buffer ), _curPos( 0 ), _failed( false ) {}
	BufferInputStream( std::vector< uint8_t > && buffer ) : _buffer( move(buffer) ), _curPos( 0 ), _failed( false ) {}
	~BufferInputStream() {}

	void reset( const std::vector< uint8_t > & buffer ) { _buffer = buffer; _curPos = 0; _failed = false; }
	void reset( std::vector< uint8_t > && buffer ) { _buffer = move(buffer); _curPos = 0; _failed = false; }

	uint8_t get()
	{
		if (canRead( 1 ))
		{
			return _buffer[ _curPos++ ];
		}
		else
		{
			return 0;
		}
	}

	/** Reads a fundamental type from the buffer, maintaining its original byte order (no endianity conversion for integers). */
	template< typename Type, typename std::enable_if< std::is_fundamental<Type>::value, int >::type = 0 >
	BufferInputStream & operator>>( Type & x )
	{
		if (canRead( sizeof(x) ))
		{
			std::copy( _buffer.begin() + _curPos, _buffer.begin() + _curPos + sizeof(x), (uint8_t *)&x );
			_curPos += sizeof(x);
		}
		return *this;
	}

	/** Reads a string from the buffer until a null terminator is found. */
	BufferInputStream & operator>>( std::string & str )
	{
		readString0( str );
		return *this;
	}

	/** Reads an arbitrary integral number from the buffer and converts it from big endian to native format.
	  * (return value variant) */
	template< typename IntType, typename std::enable_if< std::is_integral<IntType>::value, int >::type = 0 >
	IntType readIntBE()
	{
		IntType native = 0;

		if (canRead( sizeof(IntType) ))
		{
			const constByteVecIter bigEndianBegin = _buffer.begin() + _curPos;
			const constByteVecIter bigEndianEnd = bigEndianBegin + sizeof(IntType);

			constByteVecIter pos = bigEndianBegin;
			while (pos < bigEndianEnd) {
				native = (IntType)(native << 8);
				native = (IntType)(native | *pos);
				++pos;
			}

			_curPos += sizeof(IntType);
		}

		return native;
	}

	/** Reads an arbitrary integral number from the buffer and converts it from big endian to native format.
	  * (output parameter variant) */
	template< typename IntType, typename std::enable_if< std::is_integral<IntType>::value, int >::type = 0 >
	bool readIntBE( IntType & native )
	{
		native = readIntBE< IntType >();
		return !_failed;
	}

	/** Reads an arbitrary integral number from the buffer and converts it from little endian to native format.
	  * (return value variant) */
	template< typename IntType, typename std::enable_if< std::is_integral<IntType>::value, int >::type = 0 >
	IntType readIntLE()
	{
		IntType native = 0;

		if (canRead( sizeof(IntType) ))
		{
			const constByteVecIter littleEndianBegin = _buffer.begin() + _curPos;
			const constByteVecIter littleEndianEnd = littleEndianBegin + sizeof(IntType);

			constByteVecIter pos = littleEndianEnd;
			while (pos > littleEndianBegin) { // can't use traditional for-loop approach, because decrementing past begin
				--pos;                        // is formally undefined and causes assertion in some standard lib implementations
				native = (IntType)(native << 8);
				native = (IntType)(native | *pos);
			}

			_curPos += sizeof(IntType);
		}

		return native;
	}

	/** Reads an arbitrary integral number from the buffer and converts it from little endian to native format.
	  * (output parameter variant) */
	template< typename IntType, typename std::enable_if< std::is_integral<IntType>::value, int >::type = 0 >
	bool readIntLE( IntType & native )
	{
		native = readIntLE< IntType >();
		return !_failed;
	}

	/** Reads an integer representation of an enum value from the buffer and converts it from big endian to native format.
	  * (return value variant) */
	template< typename EnumType, typename std::enable_if< std::is_enum<EnumType>::value, int >::type = 0 >
	EnumType readEnumBE()
	{
		return EnumType( readIntBE< typename std::underlying_type< EnumType >::type >() );
	}

	/** Reads an integer representation of an enum value from the buffer and converts it from big endian to native format.
	  * (output parameter variant) */
	template< typename EnumType, typename std::enable_if< std::is_enum<EnumType>::value, int >::type = 0 >
	bool readEnumBE( EnumType & native )
	{
		native = readEnumBE< EnumType >();
		return !_failed;
	}

	/** Reads an integer representation of an enum value from the buffer and converts it from little endian to native format.
	  * (return value variant) */
	template< typename EnumType, typename std::enable_if< std::is_enum<EnumType>::value, int >::type = 0 >
	EnumType readEnumLE()
	{
		return EnumType( readIntLE< typename std::underlying_type< EnumType >::type >() );
	}

	/** Reads an integer representation of an enum value from the buffer and converts it from little endian to native format.
	  * (output parameter variant) */
	template< typename EnumType, typename std::enable_if< std::is_enum<EnumType>::value, int >::type = 0 >
	bool readEnumLE( EnumType & native )
	{
		native = readEnumLE< EnumType >();
		return !_failed;
	}

	/** Reads a string of specified size from the buffer.
	  * (output parameter variant) */
	bool readString( std::string & str, size_t size );

	/** Reads a string of specified size from the buffer.
	  * (return value variant) */
	std::string readString( size_t size )
	{
		std::string str;
		readString( str, size );
		return str;
	}

	/** Reads a string from the buffer until a null terminator is found.
	  * (output parameter variant) */
	bool readString0( std::string & str );

	/** Reads a string from the buffer until a null terminator is found.
	  * (return value variant) */
	std::string readString0()
	{
		std::string str;
		readString0( str );
		return str;
	}

	// TODO: read arbitrary number of characters or bytes

	void setFailed() { _failed = true; }
	void resetFailed() { _failed = false; }
	bool hasFailed() const { return _failed; }

 private:

	bool canRead( size_t size )
	{
		// the _failed flag can be true already from the previous call, in that case it will stay failed
		_failed |= _curPos + size > _buffer.size();
		return !_failed;
	}

 private:

	std::vector< uint8_t > _buffer;
	size_t _curPos;
	bool _failed;

};


//======================================================================================================================


} // namespace orgb


#endif // OPENRGB_BUFFER_STREAM_INCLUDED
