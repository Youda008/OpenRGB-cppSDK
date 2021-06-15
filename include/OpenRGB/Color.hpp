//======================================================================================================================
// Project: OpenRGB - C++ SDK
//----------------------------------------------------------------------------------------------------------------------
// Author:      Jan Broz (Youda008)
// Description: Representation of a color
//======================================================================================================================

#ifndef OPENRGB_COLOR_INCLUDED
#define OPENRGB_COLOR_INCLUDED


#include <cstddef>
#include <cstdint>

namespace own {
	class BinaryOutputStream;
	class BinaryInputStream;
}


namespace orgb {


//======================================================================================================================
/** Simple representation of a color with 3 8-bit values for red, green, blue components */

class Color
{

 public:

	uint8_t r;
	uint8_t g;
	uint8_t b;

	Color() {}
	Color( uint8_t red, uint8_t green, uint8_t blue ) : r( red ), g( green ), b( blue ) {}
	~Color() {}

	static const Color Black;
	static const Color White;
	static const Color Red;
	static const Color Green;
	static const Color Blue;
	static const Color Yellow;
	static const Color Magenta;
	static const Color Cyan;

	/** this is here only for the templates in Client implementation */
	constexpr size_t calcSize() const { return sizeof(r) + sizeof(g) + sizeof(b) + 1; }
	void serialize( own::BinaryOutputStream & stream ) const;
	bool deserialize( own::BinaryInputStream & stream );

};

void print( const Color & color );


//======================================================================================================================


} // namespace orgb


#endif // OPENRGB_COLOR_INCLUDED
