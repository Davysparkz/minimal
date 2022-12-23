namespace w32 {
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>

	// OPAQUE types
    using uint_t = UINT;
    using dword_t = DWORD;
    using handle_t = HANDLE;
    using hwnd_t = HWND;
    using rect_t = RECT;
    using hinstance_t = HINSTANCE;
    using wparam_t = WPARAM;
    using lparam_t = LPARAM;
    using hdc_t = HDC;
    using lresult_t = LRESULT;
    using hresult_t = HRESULT;
    using hbrush_t = HBRUSH;
	using atom_t = ATOM;

	// STRUCTS
	using wndclassexw_s = WNDCLASSEXW;
	using wndclassex_s = WNDCLASSEX;
	using msg_s = MSG;

}