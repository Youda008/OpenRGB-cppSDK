// HACK: This definition needs to be copied here from SystemErrorInfo.hpp, because that header is now in a submodule,
// and it isn't visible to the library users that add this to their include directories

#include <cstdint>

#ifdef _WIN32
	using system_error_t = uint32_t;  // should be DWORD but let's not include the whole windows.h just because of this
#else
	using system_error_t = int;
#endif
