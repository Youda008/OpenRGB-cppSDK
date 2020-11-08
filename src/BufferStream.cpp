//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Created on:  2.11.2020
// Description: classes for binary serialization into binary buffers via operators << and >>
//======================================================================================================================

#include "BufferStream.hpp"

#include <algorithm>  // find, copy
using std::find;
using std::copy;
using std::string;


namespace orgb {


//----------------------------------------------------------------------------------------------------------------------
//  string

void BufferOutputStream::writeString( const string & str )
{
	_buffer.resize( _buffer.size() + str.size() );
	copy( str.begin(), str.end(), _buffer.end() - str.size() );
}

void BufferOutputStream::writeString0( const string & str )
{
	_buffer.resize( _buffer.size() + str.size() + 1 );
	copy( str.begin(), str.end() + 1, _buffer.end() - str.size() - 1 );
}

bool BufferInputStream::readString( string & str, size_t size )
{
	if (canRead( size ))
	{
		str.resize( size, '0' );
		copy( _buffer.begin() + _curPos, _buffer.begin() + _curPos + size, str.begin() );
		_curPos += size;
	}
	return !_failed;
}

bool BufferInputStream::readString0( string & str )
{
	if (!_failed)
	{
		const auto curIter = _buffer.begin() + _curPos;
		const auto endIter = _buffer.end();
		const auto strEndIter = find( curIter, endIter, '\0' );
		if (strEndIter >= endIter) {
			_failed = true;
		} else {
			str.resize( strEndIter - curIter, '0' );
			copy( curIter, strEndIter, str.begin() );
			_curPos += str.size() + 1;
		}
	}
	return !_failed;
}


//----------------------------------------------------------------------------------------------------------------------


} // namespace orgb
